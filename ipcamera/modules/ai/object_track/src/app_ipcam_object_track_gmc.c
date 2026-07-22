#include "app_ipcam_object_track_gmc.h"

#include <math.h>
#include <stdint.h>
#include <string.h>
#include <time.h>

#include "cvi_sys.h"

#define GMC_SEARCH_RADIUS 8
#define GMC_COARSE_STEP_X 8
#define GMC_COARSE_STEP_Y 4
#define GMC_REFINE_STEP_X 4
#define GMC_REFINE_STEP_Y 2
#define GMC_MIN_VALID_SAMPLES 128
#define GMC_MIN_CONFIDENCE 0.08f
#define GMC_MIN_MOTION_PIXELS 1.5f
#define GMC_MAX_SEQUENCE_GAP 8

static CVI_U64 gmc_time_us(CVI_VOID)
{
    struct timespec ts;

    if (clock_gettime(CLOCK_MONOTONIC, &ts) != 0) {
        return 0;
    }
    return (CVI_U64)ts.tv_sec * 1000ULL * 1000ULL +
           (CVI_U64)ts.tv_nsec / 1000ULL;
}

static CVI_BOOL gmc_inside_box(CVI_S32 x, CVI_S32 y,
                               const CVI_S32 box[4])
{
    return box != NULL && box[2] > box[0] && box[3] > box[1] &&
           x >= box[0] && x <= box[2] && y >= box[1] && y <= box[3];
}

static CVI_VOID gmc_scale_exclusion(const CVI_S32 input[4],
                                    CVI_U32 width, CVI_U32 height,
                                    CVI_S32 output[4])
{
    CVI_S32 expand_x;
    CVI_S32 expand_y;

    memset(output, 0, sizeof(CVI_S32) * 4);
    if (input == NULL || input[2] <= input[0] || input[3] <= input[1] ||
        width == 0 || height == 0) {
        return;
    }

    output[0] = input[0] * APP_OBJECT_TRACK_GMC_GRID_WIDTH / (CVI_S32)width;
    output[1] = input[1] * APP_OBJECT_TRACK_GMC_GRID_HEIGHT / (CVI_S32)height;
    output[2] = input[2] * APP_OBJECT_TRACK_GMC_GRID_WIDTH / (CVI_S32)width;
    output[3] = input[3] * APP_OBJECT_TRACK_GMC_GRID_HEIGHT / (CVI_S32)height;
    expand_x = (output[2] - output[0]) / 5 + 1;
    expand_y = (output[3] - output[1]) / 5 + 1;
    output[0] = output[0] > expand_x ? output[0] - expand_x : 0;
    output[1] = output[1] > expand_y ? output[1] - expand_y : 0;
    output[2] += expand_x;
    output[3] += expand_y;
    if (output[2] >= APP_OBJECT_TRACK_GMC_GRID_WIDTH) {
        output[2] = APP_OBJECT_TRACK_GMC_GRID_WIDTH - 1;
    }
    if (output[3] >= APP_OBJECT_TRACK_GMC_GRID_HEIGHT) {
        output[3] = APP_OBJECT_TRACK_GMC_GRID_HEIGHT - 1;
    }
}

static CVI_FLOAT gmc_build_grid(const VIDEO_FRAME_INFO_S *frame,
                                CVI_U8 *grid)
{
    const VIDEO_FRAME_S *video = &frame->stVFrame;
    CVI_U64 sum = 0;
    CVI_U32 grid_x;
    CVI_U32 grid_y;

    for (grid_y = 0; grid_y < APP_OBJECT_TRACK_GMC_GRID_HEIGHT; grid_y++) {
        CVI_U32 y = ((grid_y * 2 + 1) * video->u32Height) /
                    (APP_OBJECT_TRACK_GMC_GRID_HEIGHT * 2);
        const CVI_U8 *row = video->pu8VirAddr[0] +
                            y * video->u32Stride[0];

        for (grid_x = 0; grid_x < APP_OBJECT_TRACK_GMC_GRID_WIDTH; grid_x++) {
            CVI_U32 x = ((grid_x * 2 + 1) * video->u32Width) /
                        (APP_OBJECT_TRACK_GMC_GRID_WIDTH * 2);
            CVI_U32 next_x = x + 1 < video->u32Width ? x + 1 : x;
            CVI_U32 value = ((CVI_U32)row[x] + row[next_x]) / 2;

            grid[grid_y * APP_OBJECT_TRACK_GMC_GRID_WIDTH + grid_x] =
                (CVI_U8)value;
            sum += value;
        }
    }

    return (CVI_FLOAT)sum /
           (APP_OBJECT_TRACK_GMC_GRID_WIDTH *
            APP_OBJECT_TRACK_GMC_GRID_HEIGHT);
}

static CVI_U64 gmc_cost(const CVI_U8 *previous, const CVI_U8 *current,
                        CVI_FLOAT previous_mean, CVI_FLOAT current_mean,
                        CVI_S32 shift_x, CVI_S32 shift_y,
                        CVI_S32 step_x, CVI_S32 step_y,
                        const CVI_S32 previous_exclusion[4],
                        const CVI_S32 current_exclusion[4],
                        CVI_U32 *valid_samples)
{
    CVI_U64 cost = 0;
    CVI_U32 count = 0;
    CVI_S32 y;

    for (y = GMC_SEARCH_RADIUS; y < APP_OBJECT_TRACK_GMC_GRID_HEIGHT -
         GMC_SEARCH_RADIUS; y += step_y) {
        CVI_S32 x;
        for (x = GMC_SEARCH_RADIUS; x < APP_OBJECT_TRACK_GMC_GRID_WIDTH -
             GMC_SEARCH_RADIUS; x += step_x) {
            CVI_S32 previous_x = x + shift_x;
            CVI_S32 previous_y = y + shift_y;
            CVI_S32 diff;

            if (gmc_inside_box(x, y, current_exclusion) ||
                gmc_inside_box(previous_x, previous_y,
                               previous_exclusion)) {
                continue;
            }
            diff = (CVI_S32)((CVI_FLOAT)current[
                       y * APP_OBJECT_TRACK_GMC_GRID_WIDTH + x] -
                       current_mean -
                       ((CVI_FLOAT)previous[
                       previous_y * APP_OBJECT_TRACK_GMC_GRID_WIDTH +
                       previous_x] - previous_mean));
            if (diff < 0) {
                diff = -diff;
            }
            if (diff > 64) {
                diff = 64;
            }
            cost += (CVI_U32)diff;
            count++;
        }
    }

    if (valid_samples != NULL) {
        *valid_samples = count;
    }
    return count >= GMC_MIN_VALID_SAMPLES ? cost / count : UINT64_MAX;
}

static CVI_FLOAT gmc_subpixel(CVI_U64 negative, CVI_U64 center,
                              CVI_U64 positive)
{
    double denominator;
    double offset;

    if (negative == UINT64_MAX || center == UINT64_MAX ||
        positive == UINT64_MAX) {
        return 0.0f;
    }
    denominator = (double)negative - 2.0 * (double)center +
                  (double)positive;
    if (fabs(denominator) < 1.0) {
        return 0.0f;
    }
    offset = 0.5 * ((double)negative - (double)positive) / denominator;
    if (offset < -0.5) {
        offset = -0.5;
    } else if (offset > 0.5) {
        offset = 0.5;
    }
    return (CVI_FLOAT)offset;
}

CVI_VOID app_ipcam_ObjectTrackGmc_Reset(
    APP_OBJECT_TRACK_GMC_STATE_S *state)
{
    if (state != NULL) {
        memset(state, 0, sizeof(*state));
    }
}

CVI_S32 app_ipcam_ObjectTrackGmc_Process(
    APP_OBJECT_TRACK_GMC_STATE_S *state,
    const VIDEO_FRAME_INFO_S *frame,
    CVI_U32 interval,
    const CVI_S32 exclusion[4],
    APP_OBJECT_TRACK_GMC_RESULT_S *result)
{
    VIDEO_FRAME_INFO_S readable_frame;
    CVI_VOID *mapped_address = NULL;
    CVI_U32 luma_length;
    CVI_U64 start_us;
    CVI_U64 stage_start_us;
    CVI_FLOAT previous_mean = 0.0f;
    CVI_FLOAT current_mean;
    CVI_U64 best_cost = UINT64_MAX;
    CVI_U64 second_cost = UINT64_MAX;
    CVI_U64 zero_cost;
    CVI_S32 current_exclusion[4];
    CVI_S32 best_x = 0;
    CVI_S32 best_y = 0;
    CVI_S32 coarse_x;
    CVI_S32 coarse_y;
    CVI_U32 best_samples = 0;
    CVI_S32 shift_y;

    if (state == NULL || frame == NULL || result == NULL) {
        return CVI_FAILURE;
    }
    memset(result, 0, sizeof(*result));
    if (interval == 0) {
        interval = 1;
    }
    state->input_count++;
    if (state->has_previous && state->input_count % interval != 0) {
        return CVI_SUCCESS;
    }

    if (frame->stVFrame.enPixelFormat != PIXEL_FORMAT_NV12 ||
        frame->stVFrame.u64PhyAddr[0] == 0 ||
        frame->stVFrame.u32Stride[0] == 0 ||
        frame->stVFrame.u32Width == 0 || frame->stVFrame.u32Height == 0) {
        return CVI_FAILURE;
    }

    start_us = gmc_time_us();
    result->evaluated = CVI_TRUE;
    readable_frame = *frame;
    luma_length = frame->stVFrame.u32Stride[0] *
                  frame->stVFrame.u32Height;
    if (readable_frame.stVFrame.pu8VirAddr[0] == NULL) {
        mapped_address = CVI_SYS_MmapCache(
            frame->stVFrame.u64PhyAddr[0], luma_length);
        if (mapped_address == NULL) {
            result->total_us = gmc_time_us() - start_us;
            return CVI_FAILURE;
        }
        readable_frame.stVFrame.pu8VirAddr[0] = mapped_address;
    }
    stage_start_us = start_us;
    if (CVI_SYS_IonInvalidateCache(
            readable_frame.stVFrame.u64PhyAddr[0],
            readable_frame.stVFrame.pu8VirAddr[0],
            luma_length) != CVI_SUCCESS) {
        if (mapped_address != NULL) {
            CVI_SYS_Munmap(mapped_address, luma_length);
        }
        result->total_us = gmc_time_us() - start_us;
        return CVI_FAILURE;
    }
    result->cache_us = gmc_time_us() - stage_start_us;

    stage_start_us = gmc_time_us();
    current_mean = gmc_build_grid(&readable_frame, state->current);
    gmc_scale_exclusion(exclusion, readable_frame.stVFrame.u32Width,
                        readable_frame.stVFrame.u32Height,
                        current_exclusion);
    result->grid_us = gmc_time_us() - stage_start_us;

    if (!state->has_previous ||
        frame->stVFrame.u32SeqenceNo <= state->previous_sequence ||
        frame->stVFrame.u32SeqenceNo - state->previous_sequence >
            GMC_MAX_SEQUENCE_GAP) {
        memcpy(state->previous, state->current, sizeof(state->previous));
        memcpy(state->previous_exclusion, current_exclusion,
               sizeof(state->previous_exclusion));
        state->previous_sequence = frame->stVFrame.u32SeqenceNo;
        state->has_previous = CVI_TRUE;
        if (mapped_address != NULL) {
            CVI_SYS_Munmap(mapped_address, luma_length);
        }
        result->total_us = gmc_time_us() - start_us;
        return CVI_SUCCESS;
    }

    {
        CVI_U64 sum = 0;
        CVI_U32 index;
        for (index = 0; index < sizeof(state->previous); index++) {
            sum += state->previous[index];
        }
        previous_mean = (CVI_FLOAT)sum / sizeof(state->previous);
    }

    stage_start_us = gmc_time_us();
    for (shift_y = -GMC_SEARCH_RADIUS; shift_y <= GMC_SEARCH_RADIUS;
         shift_y++) {
        CVI_S32 shift_x;
        for (shift_x = -GMC_SEARCH_RADIUS; shift_x <= GMC_SEARCH_RADIUS;
             shift_x++) {
            CVI_U64 cost = gmc_cost(
                state->previous, state->current, previous_mean, current_mean,
                shift_x, shift_y, GMC_COARSE_STEP_X, GMC_COARSE_STEP_Y,
                state->previous_exclusion, current_exclusion, NULL);
            if (cost < best_cost) {
                best_cost = cost;
                best_x = shift_x;
                best_y = shift_y;
            }
        }
    }

    coarse_x = best_x;
    coarse_y = best_y;
    best_cost = UINT64_MAX;
    for (shift_y = -1; shift_y <= 1; shift_y++) {
        CVI_S32 shift_x;
        for (shift_x = -1; shift_x <= 1; shift_x++) {
            CVI_S32 candidate_x = coarse_x + shift_x;
            CVI_S32 candidate_y = coarse_y + shift_y;
            CVI_U32 samples = 0;
            CVI_U64 cost;

            if (candidate_x < -GMC_SEARCH_RADIUS ||
                candidate_x > GMC_SEARCH_RADIUS ||
                candidate_y < -GMC_SEARCH_RADIUS ||
                candidate_y > GMC_SEARCH_RADIUS) {
                continue;
            }
            cost = gmc_cost(
                state->previous, state->current, previous_mean, current_mean,
                candidate_x, candidate_y, GMC_REFINE_STEP_X,
                GMC_REFINE_STEP_Y, state->previous_exclusion,
                current_exclusion, &samples);
            if (cost < best_cost) {
                second_cost = best_cost;
                best_cost = cost;
                best_x = candidate_x;
                best_y = candidate_y;
                best_samples = samples;
            } else if (cost < second_cost) {
                second_cost = cost;
            }
        }
    }
    zero_cost = gmc_cost(
        state->previous, state->current, previous_mean, current_mean, 0, 0,
        GMC_REFINE_STEP_X, GMC_REFINE_STEP_Y, state->previous_exclusion,
        current_exclusion, NULL);
    result->search_us = gmc_time_us() - stage_start_us;

    if (best_cost != UINT64_MAX && second_cost != UINT64_MAX &&
        zero_cost != UINT64_MAX && zero_cost > 0) {
        CVI_FLOAT improvement = zero_cost > best_cost ?
            (CVI_FLOAT)(zero_cost - best_cost) / zero_cost : 0.0f;
        CVI_FLOAT separation = second_cost > best_cost && second_cost > 0 ?
            (CVI_FLOAT)(second_cost - best_cost) / second_cost : 0.0f;
        CVI_FLOAT sub_x = 0.0f;
        CVI_FLOAT sub_y = 0.0f;
        CVI_S32 local_x = best_x;
        CVI_S32 local_y = best_y;

        if (local_x > -GMC_SEARCH_RADIUS && local_x < GMC_SEARCH_RADIUS &&
            local_y > -GMC_SEARCH_RADIUS && local_y < GMC_SEARCH_RADIUS) {
            CVI_U64 left = gmc_cost(
                state->previous, state->current, previous_mean, current_mean,
                local_x - 1, local_y, GMC_REFINE_STEP_X, GMC_REFINE_STEP_Y,
                state->previous_exclusion, current_exclusion, NULL);
            CVI_U64 right = gmc_cost(
                state->previous, state->current, previous_mean, current_mean,
                local_x + 1, local_y, GMC_REFINE_STEP_X, GMC_REFINE_STEP_Y,
                state->previous_exclusion, current_exclusion, NULL);
            CVI_U64 up = gmc_cost(
                state->previous, state->current, previous_mean, current_mean,
                local_x, local_y - 1, GMC_REFINE_STEP_X, GMC_REFINE_STEP_Y,
                state->previous_exclusion, current_exclusion, NULL);
            CVI_U64 down = gmc_cost(
                state->previous, state->current, previous_mean, current_mean,
                local_x, local_y + 1, GMC_REFINE_STEP_X, GMC_REFINE_STEP_Y,
                state->previous_exclusion, current_exclusion, NULL);
            sub_x = gmc_subpixel(left, best_cost, right);
            sub_y = gmc_subpixel(up, best_cost, down);
        }

        result->dx = -(best_x + sub_x) * frame->stVFrame.u32Width /
                     APP_OBJECT_TRACK_GMC_GRID_WIDTH;
        result->dy = -(best_y + sub_y) * frame->stVFrame.u32Height /
                     APP_OBJECT_TRACK_GMC_GRID_HEIGHT;
        result->confidence = improvement * 0.75f + separation * 0.25f;
        result->sampled_points = best_samples;
        result->valid = result->confidence >= GMC_MIN_CONFIDENCE &&
            (fabsf(result->dx) >= GMC_MIN_MOTION_PIXELS ||
             fabsf(result->dy) >= GMC_MIN_MOTION_PIXELS);
    }

    memcpy(state->previous, state->current, sizeof(state->previous));
    memcpy(state->previous_exclusion, current_exclusion,
           sizeof(state->previous_exclusion));
    state->previous_sequence = frame->stVFrame.u32SeqenceNo;
    if (mapped_address != NULL) {
        CVI_SYS_Munmap(mapped_address, luma_length);
    }
    result->total_us = gmc_time_us() - start_us;
    return CVI_SUCCESS;
}
