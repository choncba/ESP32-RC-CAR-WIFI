var gateway = `ws://${window.location.hostname}/ws`;
var websocket;
function initWebSocket() {
  console.log('Trying to open a WebSocket connection...');
  websocket = new WebSocket(gateway);
  websocket.onopen    = onOpen;
  websocket.onclose   = onClose;
  websocket.onmessage = onMessage; // <-- add this line
}
function onOpen(event) {
  console.log('Connection opened');
}

function onClose(event) {
  console.log('Connection closed');
  setTimeout(initWebSocket, 2000);
}
function onMessage(event) {
  var state;
  if (event.data == "1"){
    state = "ON";
  }
  else{
    state = "OFF";
  }
  document.getElementById('state').innerHTML = state;
}

window.addEventListener('load', onLoad);

function sleep(ms) {
  return new Promise(resolve => setTimeout(resolve, ms));
}

function onLoad(event) {
  initWebSocket();
  initButton();
  
  // Asi logro que lo mande cuando cambia, pero se va de viaje el esp sin el setTimeout(100)
  // Object.defineProperty(StickStatus,'x',{
  //   set: function(value){
  //     this._x = value;
  //     websocket.send(JSON.stringify({x:StickStatus._xPosition,y:StickStatus._yPosition}));
  //     setTimeout(100);
  //   }
  // });

  // Object.defineProperty(StickStatus,'y',{
  //   set: function(value){
  //     this._y = value;
  //     websocket.send(JSON.stringify({x:StickStatus._x,y:StickStatus._y}));
  //     setTimeout(100);
  //   }
  // });
  // setInterval(function(){ sendJoyData(); }, 100);

  setInterval(function(){ sendSliderData(); }, 100);
}

function initButton() {
  document.getElementById('button').addEventListener('click', toggle);
}

function toggle(){
  websocket.send('toggle');
}

function sendJoyData(){
  websocket.send(JSON.stringify({x:StickStatus.x,y:StickStatus.y}));
}

function sendSliderData(){
  var sliderDir = document.getElementById("sliderDir");
  var sliderSent = document.getElementById("sliderSent");
  websocket.send(JSON.stringify({x:sliderDir.value,y:sliderSent.value}));
}

