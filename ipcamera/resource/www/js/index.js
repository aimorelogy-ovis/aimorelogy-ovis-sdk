$(function () {
  document.getElementById("crop_canvas").style.display = "none";

  // 码流获取
  $.get('/cgi/get_cur_chn.cgi', function (data, status) {
    console.log(status);
    const obj = JSON.parse(data);
    $("#switch_stream").val(obj.current_chn);
  });

  // 截图
  $("#capture").click(function () {
    $.get('/cgi/takePhoto');
  });

  // 码流切换
  $("#switch_stream").change(function () {
    const selectedChannel = this.value;
    $.get(`/cgi/switch_stream.cgi?chn=${selectedChannel}`, function () {
      window.location.reload();
    });
  });

  // 初始化变量
  let wsUrl = null;
  let lockReconnect = false;
  let webSocket = null;
  let hiddened = false;
  let jmuxer = null;
  let curr_stream = '0'; // 默认主码流
  let frameCount = 0;
  let needReset = 0;
  let lastfpsupdate = Date.now();
  const streamSelector = document.getElementById('switch_stream');

  const canvas = document.getElementById("canvas_overlay");
  const ctx = canvas.getContext("2d");
  ctx.fillStyle = "#FF0000";
  ctx.font = "30px Arial";
  ctx.textBaseline = "middle";

  // 初始化 JMuxer
  function initializeJMuxer() {
    if (jmuxer) {
      jmuxer.destroy();
    }
    jmuxer = new JMuxer({
      node: 'player',
      mode: 'video',
      flushingTime: 40,
      fps: 25,
      clearBuffer: true,
      debug: false,
    });
  }

  initializeJMuxer();

  document.addEventListener('visibilitychange', function () {
    hiddened = document.hidden;
  });

  streamSelector.addEventListener('change', function () {

    const selectedStream = streamSelector.value;
    if (selectedStream === '0' && curr_stream !== '0') {
      curr_stream = '0';
      webSocket.send('main_stream');
    } else if (selectedStream === '1' && curr_stream !== '1') {
      curr_stream = '1';
      webSocket.send('sub_stream');
    }

    const selectedChannel = this.value;
    $.get(`/cgi/switch_stream.cgi?chn=${selectedChannel}`, function () {
      window.location.reload();
    });
  });

  $.get('/cgi/get_ws_addr.cgi', function (data) {
    console.log("Connecting to WebSocket, address: " + data);

    if (data === "ws://0.0.0.0:8000") {
      alert("只支持单路播放");
      return;
    }

    if (webSocket) {
      webSocket.close();
      webSocket = null;
    }

    wsUrl = data;
    webSocket = new WebSocket(wsUrl);
    webSocket.binaryType = 'arraybuffer';
    webSocketInit(wsUrl);
  });

  function webSocketInit(url) {
    webSocket.onopen = function () {
      console.log('WebSocket connection opened.');
      heartCheck.start();
    };

    webSocket.onerror = function (evt) {
      console.error('WebSocket error:', evt.message);
      reconnect(url);
    };

    webSocket.onclose = function () {
      console.log("WebSocket connection closed.");
      reconnect(url);
    };

    webSocket.onmessage = function (evt) {
      const buffer = new Uint8Array(evt.data);
      const type = buffer[0];
      heartCheck.start();

      if (hiddened) {
        needReset = 1;
        return;
      }

      if (needReset) {
        needReset = 0;
        jmuxer.reset();
      }

      if (type === 0) {
        // 视频帧处理
        const now = Date.now();
        frameCount++;
        if ((now - lastfpsupdate) >= 1000) {
          ctx.clearRect(canvas.width - 200, 0, 200, 50);
          ctx.fillStyle = "#FF0000";
          ctx.font = "20px Arial";
          ctx.textAlign = "right";
          ctx.fillText(`视频帧率: ${frameCount}`, canvas.width - 10, 30);
          frameCount = 0;
          lastfpsupdate = now;
        }

        jmuxer.feed({ video: buffer });
      } else if (type === 1 && streamSelector.value === '0') {
        ctx.clearRect(canvas.width - 200, 60, 200, 150);
        ctx.fillStyle = "#66CCFF";
        ctx.font = "20px Arial";
        ctx.textAlign = "right";
        // console.log("AI帧数据",buffer,buffer[1], buffer[3], buffer[5]);
        ctx.fillText("PD帧率:" +String.fromCharCode.apply(null, buffer.slice(1,2)), canvas.width - 10, 90);
        ctx.fillText("MD帧率:" +String.fromCharCode.apply(null, buffer.slice(3,4)), canvas.width - 10, 120);
        ctx.fillText("PD入侵:" +String.fromCharCode.apply(null, buffer.slice(5,6)), canvas.width - 10, 150);
      }
    };
  }

  function reconnect(url) {
    if (lockReconnect) return;
    lockReconnect = true;

    setTimeout(() => {
      createWebSocket(url);
      lockReconnect = false;
    }, 4000);
  }

  function createWebSocket(url) {
    try {
      webSocket = new WebSocket(url);
      webSocket.binaryType = 'arraybuffer';
      webSocketInit(url);
    } catch (e) {
      console.error("WebSocket creation failed:", e);
      reconnect(url);
    }
  }

  const heartCheck = {
    timeout: 3000,
    timeoutObj: null,
    serverTimeoutObj: null,
    start() {
      clearTimeout(this.timeoutObj);
      clearTimeout(this.serverTimeoutObj);

      this.timeoutObj = setTimeout(() => {
        console.log("Sending heartbeat...");
        webSocket?.send('ping');

        this.serverTimeoutObj = setTimeout(() => {
          console.warn("Heartbeat timeout, closing WebSocket...");
          webSocket?.close();
        }, this.timeout);
      }, this.timeout);
    },
  };
});
