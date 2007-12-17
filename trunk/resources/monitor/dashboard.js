function sendRequest(name, onReply, onError) {
  var request = window.XMLHttpRequest ? new XMLHttpRequest() : new ActiveXObject("MSXML2.XMLHTTP.3.0");
  request.open("GET", name, true);
  request.onreadystatechange = function () {
    if (request.readyState == 4) {
      if (request.status == 200) {
        if (onReply && request.responseText)
          onReply(eval(request.responseText));
      } else if (request.status == 0) {
        if (onError) onError();
      }
    }
  };
  request.send('');  
}

function requestVariable(name, onReply, onError) {
  sendRequest("vars/" + name, onReply, onError);
}

function setConnected(value) {
  var status = document.getElementById("status");
  status.innerHTML = value ? "Connected" : "Disconnected";
  status.className = value ? "statusConnected" : "statusDisconnected";
}

function createTable(mapping) {
  var keys = [ ];
  for (var key in mapping) {
    keys.push(key);
  }
  var table = document.getElementById("vars");
  for (var index in keys.sort()) {
    var key = keys[index];
    var row = table.insertRow(-1);
    var key_col = row.insertCell(0);
    key_col.className = "variableName";
    key_col.innerHTML = key;
    var value_col = row.insertCell(1);
    value_col.className = "variableValue";
    value_col.id = key + "_var_value";
  }
  hasCreatedTable = true;
}

var hasCreatedTable = false;
function updateUi(mapping) {
  if (!hasCreatedTable) createTable(mapping);
  setConnected(true);
  for (var key in mapping) {
    document.getElementById(key + "_var_value").innerHTML = mapping[key];
  }
}

function disconnectStatus() {
  setConnected(false);
}

function pageLoaded() {
  refreshVariables();
}

function refreshVariables() {
  requestVariable("all", updateUi, disconnectStatus);
  window.setTimeout(refreshVariables, 250);
}
