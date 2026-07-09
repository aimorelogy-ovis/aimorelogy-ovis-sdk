// #include "aos/aos.h"
// #include "rtos_malloc.h"
// #include "cvi_printf.h"
#include "semaphore.h"
// #include "intr_conf.h"
#include "asm/barrier.h"
#include "drv/list.h"
#include "drv/cvi_irq.h"
#include <csi_kernel.h>
#include <k_api.h>
#include "sys/prctl.h"
#include "reg_tdma.h"
#include "reg_tiu.h"
#include "tpu_platform.h"
#include "cvi_tpu_proc.h"
#include "cvi_tpu_interface.h"

// extern void __asm_invalidate_dcache_range(unsigned long start, unsigned long stop);
// extern void __asm_flush_dcache_range(unsigned long start, unsigned long stop);

static struct cvi_tpu_device *g_TpuDev = NULL;

enum tpu_submit_path {
	TPU_PATH_DESNORMAL = 0,
	TPU_PATH_DESSEC = 1,
	TPU_PATH_PIOTDMA = 2,
	TPU_PATH_MAX = 3,
};

struct cvi_list_node {
	struct device *dev;
	dlist_t list;
	uint32_t pid;
	uint32_t seq_no;
	uint32_t pio_seq_no;
	int dmabuf_fd;
	// enum dma_data_direction dma_dir;
	// struct dma_buf *dma_buf;
	// struct dma_buf_attachment *dma_attach;
	// struct sg_table *dma_sgt;
	void *dmabuf_vaddr;
	uint64_t dmabuf_paddr;
	struct tpu_tee_submit_info tee_info;
	enum tpu_submit_path tpu_path;
	int ret;
	struct tpu_tdma_pio_info pio_info;
};

struct tpu_profiling_info {
	uint32_t run_current_us;
	uint64_t run_sum_us;

	struct timespec timer_start;
	uint32_t timer_current_us;
	uint32_t usage;
};

struct tpu_suspend_info {
	pthread_mutex_t mutex_lock;
	uint8_t running_cnt;
};

static struct tpu_suspend_info tpu_suspend;

#define STORE_NPU_USAGE 1
#define STORE_INTERVAL 2
#define STORE_ENABLE 3

#define TASK_LIST_MAX 100
#define DONE_LIST_MAX 1000


void cvi_tpu_tdma_irq(int irq, void *data)
{
	struct cvi_tpu_device *ndev = data;

	platform_clear_int(ndev);
	disable_tdma_enable_bit();

	pthread_mutex_lock(&ndev->close_lock);

	if (ndev->use_count != 0) {
		platform_tdma_irq(ndev);
	}
	pthread_mutex_unlock(&ndev->close_lock);
}

static struct cvi_list_node *
get_from_done_list(struct cvi_kernel_work *kernel_work, u32 seq_no, enum tpu_submit_path path)
{
	struct cvi_list_node *node = NULL;
	struct cvi_list_node *pos;
	uint32_t current_pid = krhino_cur_task_get()->task_id;

	pthread_mutex_lock(&kernel_work->done_list_lock);

	if (path == TPU_PATH_PIOTDMA) {
		dlist_for_each_entry(&kernel_work->done_list, pos, struct cvi_list_node, list) {
			if ((pos->pid == current_pid) && (pos->pio_seq_no == seq_no)) {
				node = pos;
				break;
			}
		}
	} else {
		dlist_for_each_entry(&kernel_work->done_list, pos, struct cvi_list_node, list) {
			if ((pos->pid == current_pid) && (pos->seq_no == seq_no)) {
				node = pos;
				break;
			}
		}
	}
	pthread_mutex_unlock(&kernel_work->done_list_lock);

	return node;
}

static void remove_from_done_list(struct cvi_kernel_work *kernel_work,
				  struct cvi_list_node *node)
{
	pthread_mutex_lock(&kernel_work->done_list_lock);
	dlist_del(&node->list);
	pthread_mutex_unlock(&kernel_work->done_list_lock);
	if(NULL != node){
		free(node);
	}
}

static int cvi_tpu_submit(struct cvi_tpu_device *ndev, unsigned long arg)
{
	// int ret = 0;
	u32 task_list_count = 0;
	struct cvi_submit_dma_arg run_dmabuf_arg;
	struct cvi_list_node *node;
	struct cvi_list_node *pos;
	struct cvi_kernel_work *kernel_work;

	memcpy(&run_dmabuf_arg,
	       (struct cvi_submit_dma_arg *)arg,
	       sizeof(struct cvi_submit_dma_arg));

	kernel_work = &ndev->kernel_work;
	while (1) {
		pthread_mutex_lock(&ndev->kernel_work.task_list_lock);
		dlist_for_each_entry(&kernel_work->task_list, pos, struct cvi_list_node, list) {
			task_list_count++;
		}
		pthread_mutex_unlock(&ndev->kernel_work.task_list_lock);
		if (task_list_count > TASK_LIST_MAX) {
			tpu_printf(TPU_DBG, "too much task in task list\r\n");
			msleep(20);
		} else {
			break;
		}
	}

	tpu_printf(TPU_DBG, "cvi_tpu_submit path()\r\n");
	node = malloc(sizeof(struct cvi_list_node));
	if (!node)
		return -ENOMEM;
	memset(node, 0, sizeof(struct cvi_list_node));

	node->pid = krhino_cur_task_get()->task_id;
	node->seq_no = run_dmabuf_arg.seq_no;
	// node->dmabuf_fd = run_dmabuf_arg.addr;
	node->dmabuf_vaddr = run_dmabuf_arg.addr;
	node->dmabuf_paddr = (uintptr_t)run_dmabuf_arg.addr;
	// node->dev = ndev->dev;
	node->tpu_path = TPU_PATH_DESNORMAL;
	// cvi_tpu_prepare_buffer(node);

	pthread_mutex_lock(&ndev->kernel_work.task_list_lock);
	dlist_add_tail(&node->list, &ndev->kernel_work.task_list);
	sem_post(&(ndev->kernel_work.task_wait_sem));
	pthread_mutex_unlock(&ndev->kernel_work.task_list_lock);

	return 0;
}

static int cvi_tpu_run_dmabuf(struct cvi_tpu_device *ndev, struct cvi_list_node *node)
{
	int ret = 0;

	tpu_printf(TPU_DBG, "enter cvi_tpu_run_dmabuf()\r\n");

	pthread_mutex_lock(&ndev->dev_lock);
	platform_tpu_init(ndev);

	tpu_printf(TPU_DBG, "node->tpu_path = %d\r\n", node->tpu_path);
	switch (node->tpu_path) {
	case TPU_PATH_DESNORMAL:
		ret = platform_run_dmabuf(ndev, node->dmabuf_vaddr, node->dmabuf_paddr);
		break;
	case TPU_PATH_DESSEC:
		ret = platform_run_dmabuf_tee(ndev, &node->tee_info);
		break;
	case TPU_PATH_PIOTDMA:
		ret = platform_run_pio(ndev, &node->pio_info);
		break;
	default:
		break;
	}

	if (ret == -ETIMEDOUT) {
		platform_tpu_reset(ndev);
	} else {
		platform_tpu_deinit(ndev);
	}
	pthread_mutex_unlock(&ndev->dev_lock);
	tpu_printf(TPU_DBG, "exit cvi_tpu_run_dmabuf()\r\n");
	return ret;
}

static int cvi_tpu_wait_pio(struct cvi_tpu_device *ndev, unsigned long arg)
{
	int ret = 0;
	struct cvi_kernel_work *kernel_work = &ndev->kernel_work;
	struct cvi_tdma_wait_arg wait_pio_arg;
	struct cvi_list_node *node;

	memcpy(&wait_pio_arg,
	       (struct cvi_tdma_wait_arg *)arg,
	       sizeof(struct cvi_tdma_wait_arg));

	while (1) {
		sem_wait(&(kernel_work->done_wait_sem));
		node = get_from_done_list(kernel_work, wait_pio_arg.seq_no, TPU_PATH_PIOTDMA);
		if (node) {
			break;
		}
	}
	if (ret) {
		return -EINTR;
	}

	wait_pio_arg.ret = node->ret;
	memcpy((unsigned long *)arg,
	       (const void *)&wait_pio_arg,
	       sizeof(struct cvi_tdma_wait_arg));

	remove_from_done_list(kernel_work, node);
	return ret;
}

int cvi_tpu_submit_pio(struct cvi_tpu_device *ndev, unsigned long arg)
{
	// int ret = 0;
	struct cvi_tdma_copy_arg ioctl_arg;
	u32 task_list_count = 0;
	struct cvi_list_node *node = NULL;
	struct cvi_list_node *pos = NULL;
	struct cvi_kernel_work *kernel_work =  &ndev->kernel_work;
	// unsigned int buf_size;

	tpu_printf(TPU_DBG, "enter cvi_tpu_submit_pio\r\n");

	memcpy(&ioctl_arg,
	       (struct cvi_tdma_copy_arg *)arg,
	       sizeof(struct cvi_tdma_copy_arg));
	while (1) {
		pthread_mutex_lock(&ndev->kernel_work.task_list_lock);
		dlist_for_each_entry(&kernel_work->task_list, pos, struct cvi_list_node, list) {
			task_list_count++;
		}
		pthread_mutex_unlock(&ndev->kernel_work.task_list_lock);
		if (task_list_count > TASK_LIST_MAX) {
			tpu_printf(TPU_DBG, "too much task in task list\r\n");
			msleep(20);
		} else {
			break;
		}
	}


	node = malloc(sizeof(struct cvi_list_node));
	if (NULL == node) {
		tpu_printf(TPU_ERR, "vmalloc error\r\n");
		return -ENOMEM;
	}
	memset(node, 0, sizeof(struct cvi_list_node));

	node->pid = krhino_cur_task_get()->task_id;
	node->pio_seq_no = ioctl_arg.seq_no;
	node->tpu_path = TPU_PATH_PIOTDMA;
	if (ioctl_arg.enable_2d) {
		node->pio_info.enable_2d = 1;
		node->pio_info.paddr_src = ioctl_arg.paddr_src;
		node->pio_info.paddr_dst = ioctl_arg.paddr_dst;
		node->pio_info.h = ioctl_arg.h;
		node->pio_info.w_bytes = ioctl_arg.w_bytes;
		node->pio_info.stride_bytes_src = ioctl_arg.stride_bytes_src;
		node->pio_info.stride_bytes_dst = ioctl_arg.stride_bytes_dst;
	} else {
		node->pio_info.paddr_src = ioctl_arg.paddr_src;
		node->pio_info.paddr_dst = ioctl_arg.paddr_dst;
		node->pio_info.leng_bytes = ioctl_arg.leng_bytes;
	}

	pthread_mutex_lock(&ndev->kernel_work.task_list_lock);
	dlist_add_tail(&node->list, &ndev->kernel_work.task_list);
	sem_post(&(kernel_work->task_wait_sem));
	pthread_mutex_unlock(&ndev->kernel_work.task_list_lock);
	return 0;
}

static void cvi_tpu_cleanup_done_list(struct cvi_tpu_device *ndev,
				      struct cvi_kernel_work *kernel_work)
{
	u32 done_list_count = 0;
	struct cvi_list_node *pos;
	dlist_t *tmp;
	klist_t *p;
	ktask_t *task;
	char alive;

	pthread_mutex_lock(&kernel_work->done_list_lock);

	dlist_for_each_entry(&kernel_work->done_list, pos, struct cvi_list_node, list) {
		done_list_count++;
	}

	if (done_list_count < DONE_LIST_MAX) {
		pthread_mutex_unlock(&kernel_work->done_list_lock);
		return;
	}

	tpu_printf(TPU_DBG, "done list too much node, clean up\r\n");

	// Delete the node of the task that has ended
	dlist_for_each_entry_safe(&kernel_work->done_list, tmp, pos, struct cvi_list_node, list) {
		alive = false;
		for (p = g_kobj_list.task_head.next; p != &g_kobj_list.task_head; p = p->next) {
			task = krhino_list_entry(p, ktask_t, task_stats_item);
			if (pos->pid == task->task_id) {
				alive = true;
				break;
			}
		}
		if (!alive) {
			dlist_del(&pos->list);
			tpu_printf(TPU_DBG, "free buf\r\n");
			if(NULL != pos){
				free(pos);
			}
		}
	}

	pthread_mutex_unlock(&kernel_work->done_list_lock);
}

static void work_thread_run(struct cvi_tpu_device *ndev)
{
	struct cvi_kernel_work *kernel_work = &ndev->kernel_work;
	struct cvi_list_node *first_node;
	int ret = 0;

	pthread_mutex_lock(&kernel_work->task_list_lock);
	first_node = dlist_first_entry(&kernel_work->task_list,
				       struct cvi_list_node, list);
	dlist_del(&first_node->list);
	pthread_mutex_unlock(&kernel_work->task_list_lock);

	//before tpu inference
	tpu_suspend.running_cnt = 1;

	//tpu inference HW running process
	ret = cvi_tpu_run_dmabuf(ndev, first_node);
	first_node->ret = ret;

	//after tpu inference
	tpu_suspend.running_cnt = 0;

	pthread_mutex_lock(&kernel_work->done_list_lock);
	dlist_add_tail(&first_node->list, &kernel_work->done_list);
	pthread_mutex_unlock(&kernel_work->done_list_lock);

	sem_post(&kernel_work->done_wait_sem);

	cvi_tpu_cleanup_done_list(ndev, kernel_work);
}

static int task_list_empty(struct cvi_kernel_work *kernel_work)
{
	int ret;

	pthread_mutex_lock(&kernel_work->task_list_lock);
	ret = dlist_empty(&kernel_work->task_list);
	pthread_mutex_unlock(&kernel_work->task_list_lock);

	return ret;
}

static void work_thread_exit(struct cvi_tpu_device *ndev)
{
	struct cvi_kernel_work *kernel_work = &ndev->kernel_work;
	struct cvi_list_node *pos;
	dlist_t *tmp;

	pthread_mutex_lock(&kernel_work->task_list_lock);
	dlist_for_each_entry_safe(&kernel_work->task_list, tmp, pos, struct cvi_list_node, list) {
		dlist_del(&pos->list);
		if(NULL != pos){
			free(pos);
		}
	}
	pthread_mutex_unlock(&kernel_work->task_list_lock);

	pthread_mutex_lock(&kernel_work->done_list_lock);
	dlist_for_each_entry_safe(&kernel_work->done_list, tmp, pos, struct cvi_list_node, list) {
		dlist_del(&pos->list);
		if(NULL != pos){
			free(pos);
		}
	}
	pthread_mutex_unlock(&kernel_work->done_list_lock);
}

void *work_thread_main(void *data)
{
	struct cvi_tpu_device *ndev = (struct cvi_tpu_device *)data;
	struct cvi_kernel_work *work = &ndev->kernel_work;
	struct timespec cur_time = {0};

	tpu_printf(TPU_DBG, "enter work_thread_main()\r\n");
	prctl(PR_SET_NAME, "tpu_work_thread");
	work->work_run = 1;

	while (work->work_run) {
		while (clock_gettime(CLOCK_REALTIME, &cur_time) == -1)
			continue;
		cur_time.tv_nsec += 500 * 1000000;
		cur_time.tv_sec += cur_time.tv_nsec / 1000000000;
		cur_time.tv_nsec %= 1000000000;

		int ret = sem_timedwait(&(work->task_wait_sem), &cur_time);
		if (ret != 0) {
			continue;
		}
		if (!task_list_empty(work))
			work_thread_run(ndev);
	}

	work_thread_exit(ndev);
	tpu_printf(TPU_DBG, "exit work_thread_main()\r\n");
	return NULL;
}

static int work_thread_init(struct cvi_tpu_device *ndev)
{
	struct cvi_kernel_work *kernel_work = &(ndev->kernel_work);

	sem_init(&(kernel_work->task_wait_sem), 0, 0);
	sem_init(&(kernel_work->done_wait_sem), 0, 0);
	INIT_AOS_DLIST_HEAD(&kernel_work->task_list);
	pthread_mutex_init(&kernel_work->task_list_lock, NULL);
	INIT_AOS_DLIST_HEAD(&kernel_work->done_list);
	pthread_mutex_init(&kernel_work->done_list_lock, NULL);

	int ret = pthread_create(&(kernel_work->work_thread), NULL, work_thread_main, ndev);
	if (ret != 0) {
		tpu_printf(TPU_ERR, "kthread run fail , ret = %d , errno = %d\r\n", ret, errno);
		return ret;
	}
	tpu_printf(TPU_DBG, "pthread_create success. pthreadID = 0x%x\r\n", kernel_work->work_thread);

	return 0;
}

static int cvi_tpu_wait_dmabuf(struct cvi_tpu_device *ndev, unsigned long arg)
{
	int ret = 0;
	struct cvi_kernel_work *kernel_work = &ndev->kernel_work;
	struct cvi_wait_dma_arg wait_dmabuf_arg;
	struct cvi_list_node *node;

	memcpy(&wait_dmabuf_arg,
	       (struct cvi_wait_dma_arg *)arg,
	       sizeof(struct cvi_wait_dma_arg));

	while (1) {
		sem_wait(&(kernel_work->done_wait_sem));
		node = get_from_done_list(kernel_work, wait_dmabuf_arg.seq_no, TPU_PATH_DESNORMAL);
		if (node)
			break;
	}

	wait_dmabuf_arg.ret = node->ret;
	memcpy((unsigned long *)arg,
	       (const void *)&wait_dmabuf_arg,
	       sizeof(struct cvi_wait_dma_arg));

	remove_from_done_list(kernel_work, node);

	return ret;
}

static int cvi_tpu_cache_flush(struct cvi_tpu_device *ndev, struct cvi_cache_op_arg *flush_arg)
{
	tpu_printf(TPU_DBG, "flush_arg->paddr=0x%x, flush_arg->size=0x%x\r\n", flush_arg->paddr,
		   flush_arg->size);
	csi_dcache_clean_range((uintptr_t *) flush_arg->paddr, flush_arg->size);
	/* sync */
	__smp_mb();

	return 0;
}

static int cvi_tpu_cache_invalidate(struct cvi_tpu_device *ndev, struct cvi_cache_op_arg *invalidate_arg)
{
	tpu_printf(TPU_DBG, "invalidate_arg->paddr=0x%x, invalidate_arg->size=0x%x\r\n",
		   invalidate_arg->paddr, invalidate_arg->size);
	csi_dcache_invalid_range((uintptr_t *)invalidate_arg->paddr, invalidate_arg->size);
	tpu_printf(TPU_DBG, "invalidate_arg->paddr=0x%x, invalidate_arg->size=0x%x\r\n",
		   invalidate_arg->paddr, invalidate_arg->size);
	/*sync	*/
	__smp_mb();

	return 0;
}

int cvi_tpu_ioctl(void *dev, unsigned int cmd, unsigned long arg)
{
	int ret = 0;

	tpu_printf(TPU_DBG, "arg=0x%x \r\n", arg);

	struct cvi_tpu_device *ndev = (struct cvi_tpu_device *)dev;
	switch (cmd) {
	case CVITPU_SUBMIT_DMABUF:
		ret = cvi_tpu_submit(ndev, arg);
		break;
	// case CVITPU_DMABUF_FLUSH_FD:
	// 	ret = cvi_tpu_map_dmabuf(ndev, arg, DMA_TO_DEVICE);
	// 	break;
	// case CVITPU_DMABUF_INVLD_FD:
	// 	ret = cvi_tpu_map_dmabuf(ndev, arg, DMA_FROM_DEVICE);
	// 	break;
	case CVITPU_DMABUF_FLUSH:
		ret = cvi_tpu_cache_flush(ndev, (struct cvi_cache_op_arg *)arg);
		break;
	case CVITPU_DMABUF_INVLD:
		ret = cvi_tpu_cache_invalidate(ndev, (struct cvi_cache_op_arg *)arg);
		break;
	case CVITPU_WAIT_DMABUF:
		ret = cvi_tpu_wait_dmabuf(ndev, arg);
		break;

	// case CVITPU_LOAD_TEE:
	// 	ret = cvi_tpu_load_tee(ndev, arg);
	// 	break;
	// case CVITPU_SUBMIT_TEE:
	// 	ret = cvi_tpu_submit_tee(ndev, arg);
	// 	break;
	// case CVITPU_UNLOAD_TEE:
	// 	ret = cvi_tpu_unload_tee(ndev, arg);
	// 	break;

	case CVITPU_SUBMIT_PIO:
		ret = cvi_tpu_submit_pio(ndev, arg);
		break;
	case CVITPU_WAIT_PIO:
		ret = cvi_tpu_wait_pio(ndev, arg);
		break;

	default:
		return -ENOTTY;
	}

	return ret;
}

void *cvi_tpu_open(void)
{
	int ret;

	pthread_mutex_lock(&(g_TpuDev->close_lock));
	if (g_TpuDev->use_count == 0) {

		ret = platform_tpu_open(g_TpuDev);
		if (ret < 0) {
			tpu_printf(TPU_DBG, "npu open failed ret=%d\r\n", ret);
			return NULL;
		}
	}
	g_TpuDev->use_count++;
	pthread_mutex_unlock(&(g_TpuDev->close_lock));

	return (void *)g_TpuDev;
}

int cvi_tpu_close(void)
{
	pthread_mutex_lock(&(g_TpuDev->close_lock));
	g_TpuDev->use_count--;
	pthread_mutex_unlock(&(g_TpuDev->close_lock));

	return 0;
}

int cvi_tpu_probe(void)
{
	int ret = 0;

	tpu_printf(TPU_DBG, "===cvi_tpu_probe start\r\n");

	g_TpuDev = malloc(sizeof(struct cvi_tpu_device));
	if (g_TpuDev == NULL) {
		tpu_printf(TPU_ERR, "cvi_tpu_device kmalloc failed!\r\n");
		return -1;
	}
	tpu_printf(TPU_DBG, "kmalloc g_TpuDev(0x%lx) success!\r\n", (uintptr_t)g_TpuDev);

	g_TpuDev->tdma_paddr = TDMA_ENGINE_BASE_ADDR;
	g_TpuDev->tiu_paddr = TIU_ENGINE_BASE_ADDR;
	g_TpuDev->tdma_irq_num = TPU_IRQ_2;
	if (g_TpuDev->tdma_irq_num < 0) {
		tpu_printf(TPU_ERR, "failed to retrieve tdma irq");
		return -ENXIO;
	}

	sem_init(&g_TpuDev->tdma_done, 0, 0);
	// 初始化锁
	pthread_mutex_init(&tpu_suspend.mutex_lock, NULL);
	tpu_suspend.running_cnt = 0;
	pthread_mutex_init(&g_TpuDev->dev_lock, NULL);
	pthread_mutex_init(&g_TpuDev->close_lock, NULL);
	g_TpuDev->use_count = 0;
	//probe tpu setting
	platform_tpu_probe_setting(g_TpuDev);

	// 注册中断
	if (request_irq(g_TpuDev->tdma_irq_num, cvi_tpu_tdma_irq, 0, "cvi-tpu-tdma", g_TpuDev)) {
		tpu_printf(TPU_ERR, "Unable to request scl IRQ(%d)\r\n", g_TpuDev->tdma_irq_num);
		return -EINVAL;
	}

	ret = work_thread_init(g_TpuDev);
	if (ret < 0) {
		tpu_printf(TPU_ERR, "work thread init error\r\n");
		return ret;
	}
	tpu_printf(TPU_DBG, "exit cvi_tpu_probe\r\n");

	return 0;
}

// 实际用不到
int cvi_tpu_remove(void)
{
	if (g_TpuDev->kernel_work.work_run == 1) {
		g_TpuDev->kernel_work.work_run = 0;
		pthread_join(g_TpuDev->kernel_work.work_thread, NULL);
	}
	//pthread_cancel(g_TpuDev->kernel_work.work_thread);

	// //put clock related
	// if (ndev->clk_tpu_axi)
	// 	devm_clk_put(&pdev->dev, ndev->clk_tpu_axi);

	// if (ndev->clk_tpu_fab)
	// 	devm_clk_put(&pdev->dev, ndev->clk_tpu_fab);

	pthread_mutex_destroy(&tpu_suspend.mutex_lock);
	pthread_mutex_destroy(&g_TpuDev->dev_lock);
	pthread_mutex_destroy(&g_TpuDev->close_lock);
	pthread_mutex_destroy(&g_TpuDev->kernel_work.task_list_lock);
	pthread_mutex_destroy(&g_TpuDev->kernel_work.done_list_lock);

	free(g_TpuDev);
	g_TpuDev = NULL;
	tpu_printf(TPU_DBG, "===cvi_tpu_remove\r\n");
	return 0;
}

/*
static int cvi_tpu_suspend(void)
{
	tpu_printf(TPU_DBG, "enter cvi_tpu_suspend\r\n");
	if (tpu_suspend.running_cnt) {
		//doing register backup
		platform_tpu_suspend(g_TpuDev);
	}
	tpu_printf(TPU_DBG, "exit cvi_tpu_suspend\r\n");
	return 0;
}

static int cvi_tpu_resume(void)
{
	if (tpu_suspend.running_cnt) {
		platform_tpu_resume(g_TpuDev);
	}
	return 0;
}
*/

void cvi_tpu_init(void)
{
	if (g_TpuDev){
		tpu_printf(TPU_WARN, "tpu driver already initialized\r\n");
	}else{
		platform_tpu_init(NULL);
		cvi_tpu_probe();
	}
}
void cvi_tpu_deinit(void)
{
	if (g_TpuDev){
		platform_tpu_deinit(NULL);
		cvi_tpu_remove();
	}else{
		tpu_printf(TPU_WARN, "tpu driver already deinitialization\r\n");
	}
}