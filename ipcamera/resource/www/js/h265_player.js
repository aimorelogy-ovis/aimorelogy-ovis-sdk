var DECODER_H264 = 0;
var DECODER_H265 = 2;
var decoder_type = DECODER_H265
var webglPlayer, webglcanvas;
var LOG_LEVEL_WASM = 1;
var lockReconnect = false;
var tt = null;
var webSocket = null;

$(function () {

  var pts = 0;
  var timestamp = 0;
  var frameCount = 0;
  var chunk_size = 1024*1024;
  var wsUrl = null;
  var defaultstream = "sub_stream";

  var canvas = document.getElementById("canvas_Overlay");
  var ctx = canvas.getContext("2d");
  ctx.fillStyle = "#FF0000";
  ctx.font = "20px Arial";
  ctx.textAlign = 'right';
  ctx.textBaseline = 'top';

  $.get('/cgi/get_ws_addr.cgi', function (data, status) {
    console.log("enter connectWS, addr: " + data);
    wsUrl = data;

    console.log(wsUrl);
    webSocket = new WebSocket(wsUrl); // just for sub stream test h265
    webSocket.binaryType = 'arraybuffer';

    Module.onRuntimeInitialized = function () {
      console.log("Module initialized");
      console.log("Module exports:", Module);
      webSocketInit(wsUrl);
    };

  })

  $.get('/cgi/get_cur_chn.cgi', function (data, status) {
    console.log(status);
    const obj = JSON.parse(data);
    if(obj.current_chn == '0'){
      defaultstream = "main_stream";
      document.getElementById("stream_span").textContent = "码流选择：主码流";
    } else if(obj.current_chn == '1'){
      defaultstream = "sub_stream";
      document.getElementById("stream_span").textContent = "码流选择：子码流";
    }
  });

  function webSocketInit(data) {
    var cacheBuffer = null;

    if (typeof Module._malloc !== 'function') {
      console.error("Module._malloc is not available");
      return;
    }

    if (cacheBuffer) {
      Module._free(cacheBuffer);
    }

    if (typeof chunk_size !== 'number' || chunk_size <= 0) {
      throw new Error('Invalid chunk_size: ' + chunk_size);
    }

    webSocket.onopen = function (evt) {
      console.log('Connection open ...');
      webSocket.send(defaultstream);  // default decode sub stream in h265
      console.log("type stream: " + defaultstream);
      decode_seq(decoder_type);
    };

    webSocket.onmessage = function (evt) {
      var buffer = new Uint8Array(evt.data);
      var type = buffer[0];
      var size = buffer.length;

      if (size > chunk_size) {
        console.log("frame size out of bound");
        return;
      }

      if (type == 0) {
        var myDate = new Date().getTime();
        frameCount++;
        if ((myDate - timestamp) > 1000) {
          timestamp = myDate;
          ctx.clearRect(0, 0, canvas.width, canvas.height);
          ctx.fillText("视频帧率:" + frameCount, canvas.width - 100, 20);
          frameCount = 0;
        }

        if( typeof Module._decodeData !== 'function') {
          console.error("_decodeData is not availabel!");
        }
        try {
          cacheBuffer = Module._malloc(chunk_size);
        } catch (error) {
          console.error("Error during memory allocation:", error);
          return;
        }

        try {
          Module.HEAPU8.set(buffer, cacheBuffer);
          Module._decodeData(cacheBuffer, size , pts++);
        } catch (error) {
          console.error("Error during data decoding:", error);
        }
          Module._free(cacheBuffer);
      }
    };

    webSocket.onerror = function (evt) {
      console.log('WS error ' + evt.message);
    };

    webSocket.onclose = function () {
      console.log("WS closed");
      if (cacheBuffer) {
        Module._free(cacheBuffer);
        cacheBuffer = null;
      }
    };
  }

})

function decode_seq(decoder_type) {
  // var videoSize = 0;
  var videoCallback = Module.addFunction(function (addr_y, addr_u, addr_v, stride_y, stride_u, stride_v, width, height, pts) {
      //console.log("[%d]In video callback, size = %d * %d, pts = %d", ++videoSize, width, height, pts)
      let size = width * height;
      let y_data = HEAPU8.subarray(addr_y, addr_y + size)
      //y_data = new Uint8Array(y_data)

      let u_data = HEAPU8.subarray(addr_u, addr_u + size/2)
      //u_data = new Uint8Array(u_data)

      let v_data = HEAPU8.subarray(addr_v, addr_v + size/2)
      //v_data = new Uint8Array(v_data)

      displayVideoFrame(y_data, u_data, v_data, width, height);
  },"viiiiiiiii");

  var ret = Module._openDecoder(decoder_type, videoCallback, LOG_LEVEL_WASM)
  if(ret == 0) {
      console.log("openDecoder success");
  } else {
      console.error("openDecoder failed with error", ret);
      return;
  }
}

function displayVideoFrame(y_data, u_data, v_data, width, height) {
  if(!webglPlayer) {
      const canvasId = "player";
      webglcanvas = document.getElementById(canvasId);
      webglPlayer = new WebGLPlayer(webglcanvas, {
          preserveDrawingBuffer: false,
      });
  }
  webglPlayer.renderFrame(y_data, u_data, v_data, width, height);
}

