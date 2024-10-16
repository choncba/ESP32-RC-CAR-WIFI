var gateway = `ws://${window.location.hostname}/ws`;
var websocket;

var command = {
  up: false,
  down: false,
  left: false,
  right: false,
  lights: false,
  sound: false
}

var status = {
  battery: 0,
  signal:0
}

const pressedKeys = new Set();

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
  console.log(event.data);
}

window.addEventListener('load', onLoad);

function sleep(ms) {
  return new Promise(resolve => setTimeout(resolve, ms));
}

function onLoad(event) {
  initWebSocket();
  initPad();
  initKeys();
}

function sendCommand(buttonId, value){
  switch(buttonId){
    case "button-up": command.up = value; 
    break;
    case "button-down": command.down = value; 
    break;
    case "button-left": command.left = value; 
    break;
    case "button-right": command.right = value; 
    break;
    case "button-lights": command.lights = value; 
    break;
    case "button-sound": command.sound = value; 
    break;
    default: break;
  }
  console.log(command);
  websocket.send(JSON.stringify(command));
}

function initKeys(){
  document.addEventListener('keydown', (event) => {
    pressedKeys.add(event.key);
    HandleKeys();
  });

  document.addEventListener('keyup', (event) => {
    pressedKeys.delete(event.key);
    switch(event.key){
      case 'ArrowUp': sendCommand('button-up', false); break;
      case 'ArrowDown': sendCommand('button-down', false); break;
      case 'ArrowLeft': sendCommand('button-left', false); break;
      case 'ArrowRight': sendCommand('button-right', false); break;
      default: break;
    }
  });
}

function HandleKeys(){
  // console.log(pressedKeys);
  if(pressedKeys.has('ArrowUp') && !command.up) sendCommand('button-up', true);
  if(pressedKeys.has('ArrowDown') && !command.down) sendCommand('button-down', true);
  if(pressedKeys.has('ArrowLeft') && !command.left) sendCommand('button-left', true);
  if(pressedKeys.has('ArrowRight') && !command.right) sendCommand('button-right', true);
}

function initPad(){
  // Select all buttons with the same class
  const buttons = document.querySelectorAll('.btn');
  
  // Add a click event listener to each button
  buttons.forEach(button => {
    
    button.addEventListener('mouseup', function(event) {
      const buttonId = event.target.closest('button').getAttribute('id');
      sendCommand(buttonId, false);
      console.log(`Button ${buttonId} released`);
    });

    button.addEventListener('mousedown', function(event) {
      const buttonId = event.target.closest('button').getAttribute('id');
      sendCommand(buttonId, true);
      console.log(`Button ${buttonId} pressed`);
    });

  });
}


