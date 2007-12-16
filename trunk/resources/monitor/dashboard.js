function sendRequest(name, callback) {
  var request = window.XMLHttpRequest ? new XMLHttpRequest() : new ActiveXObject("MSXML2.XMLHTTP.3.0");
  request.open("GET", name, true);
  request.onreadystatechange = function () {
    if (request.readyState == 4 && request.status == 200) {
      if (request.responseText)
        callback(eval(request.responseText));
    }
  };
  request.send('');  
}

function requestVariable(name, callback) {
  sendRequest("vars/" + name, callback);
}

function createTable(mapping) {
  var keys = [ ];
  for (var key in mapping) {
    keys.push(key);
  }
  var table = document.createElement("table");
  table.className = "vars";
  table.id = "vars";
  document.body.appendChild(table);
  table.innerHTML = "<tr><th>Variable</th><th>Value</th></tr>";
  for (var index in keys.sort()) {
    var key = keys[index];
    var value_tuple = mapping[key];
    var chlass = value_tuple[0];
    var row = table.insertRow(-1);
    var key_col = row.insertCell(0);
    key_col.className = "variableName";
    key_col.innerHTML = chlass + "::" + key;
    var value_col = row.insertCell(1);
    value_col.className = "variableValue";
    value_col.id = key + "_var_value";
  }
}

function updateUi(mapping) {
  var table = document.getElementById("vars");
  if (!table) createTable(mapping);
  for (var key in mapping) {
    document.getElementById(key + "_var_value").innerHTML = mapping[key][1];
  }
}

function pageLoaded() {
  refreshVariables();
}

function refreshVariables() {
  requestVariable("all", updateUi);
  window.setTimeout(refreshVariables, 250);
}
