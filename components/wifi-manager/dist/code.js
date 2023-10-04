let selectedForm="ap_select";

function gel(e) {
  return document.getElementById(e);
}

function show(e) {
  gel(e).style.display = "block";
}

function hide(e) {
  gel(e).style.display = "none";
}

/* Access point form api */
function handleSelectSSID(ssid) {
  selectedForm="wifi_setup";
  hide("ap_select");
  show("wifi_setup");
  gel("navigation").style.display = "flex";

  if(ssid && ssid.length !== 0) {
    gel("wifi_ssid").value = ssid;
    gel("wifi_ssid").disabled = true;
  }else {
    gel("wifi_ssid").value = "";
    gel("wifi_ssid").disabled = false;
  }
  handleWpaChange(gel("wifi_wpa").value);
}

function handleWpaChange(wpa) {
  gel("auth_tls").style.display = "none";
  if (wpa === "personal") {
    hide("wpa_enterprise");
    show("wifi_password_visible");
    checkFormSecurity();
  }else {
    show("wpa_enterprise");
    show("auth_no_tls");
    gel("wifi_auth").value = "peap";
    handleUseUsername(gel("cb-username").checked);
    handleUseCA(gel("cb-ca").checked);
  }
}

function handleAuthChange(auth) {
  switch(auth){
    case "peap":
    case "ttls":
      hide("auth_tls");
      show("auth_no_tls");
      break;
    case "tls":
      show("auth_tls");
      hide("auth_no_tls");
      break;
  }
  checkFormSecurity();
}

function handleUseUsername(checked) {
  gel("wifi_username").disabled = !checked;
  checkFormSecurity();
}

function handleUseCA(checked) {
  gel("wifi_ca").disabled = !checked;
  checkFormSecurity();
}

function checkFormSecurity() {
  gel("next").disabled = true;
  if(gel("wifi_ssid").value.length === 0) return; 
  if(gel("wifi_wpa").value === "personal") {
    if(gel("wifi_password").value.length === 0) return;
  }else  {
    if(gel("wifi_identity").value.length === 0) return;
    if(gel("cb-username").checked && gel("wifi_username").value.length === 0) return;
    if(gel("wifi_auth").value === "tls") {
      if(gel("wifi_tls_ca").value.length === 0) return;
      if(gel("wifi_crt").value.length === 0) return;
      if(gel("wifi_key").value.length === 0) return;
    }else {
      if(gel("cb-ca").checked && gel("wifi_ca").value.length === 0) return;
      if(gel("wifi_password").value.length === 0) return;
    }
  }
  gel("next").disabled = false;
}

/* IPV4 setup form api */
function handleUseNtp(checked) {
  gel("ipv4_ntp").disabled = !checked;
  checkFormIpv4();
}

function handleIpv4MethodChange(method) {
  if (method === "auto") {
    hide("ipv4_manul");
  }else {
    show("ipv4_manul");
  }
  checkFormIpv4();
}

function checkFormIpv4() {
  gel("next").disabled = true;
  if(gel("ipv4_method") === "manual") {
    if(gel("ipv4_address").value.length === 0) return; 
    if(gel("ipv4_mask").value.length === 0) return; 
    if(gel("ipv4_gate").value.length === 0) return; 
    if(gel("ipv4_dns1").value.length === 0) return; 
  }
  if(gel("cb-ntp").checked && !(isDomainName(gel("ipv4_ntp").value) || isIpAddress(gel("ipv4_ntp").value))) return;
  gel("next").disabled = false;
}

function isIpAddress(ip) {
  return /^(25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)\.(25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)\.(25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)\.(25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)$/.test(
    ip,
  );
}

function isDomainName(name){
  return /^((([a-zA-Z]{1,2})|([0-9]{1,2})|([a-zA-Z0-9]{1,2})|([a-zA-Z0-9][a-zA-Z0-9-]{1,61}[a-zA-Z0-9]))\.)+[a-zA-Z]{2,6}$/.test(name);
}

function isNumber(char) {
  return /^\d+$/.test(char);
}

/* HTTP Server setup form api */
function handleUseEspKey(checked){
  gel("esp_json_key").disabled = !checked;
  checkFormHttpClient();
}

function handleUseStmKey(checked){
  gel("stm_json_key").disabled = !checked;
  checkFormHttpClient();
}

function handleServerAuthChange(auth) {
  switch(auth) {
    case "no":
      hide("server_auth_basic");
      hide("server_auth_tls");
      break;
    case "basic":
      show("server_auth_basic");
      hide("server_auth_tls");
      break;
    case "tls":
      hide("server_auth_basic");
      show("server_auth_tls");
      break;
  }
  checkFormHttpClient();
}

function checkFormHttpClient() {
  gel("next").disabled = true;
  if(!isIpAddress(gel("server_address").value)) return;
  if(!isNumber(gel("server_port").value)) return;
  if(gel("server_api").value.length === 0) return;
  if(gel("cb-esp").checked && gel("esp_json_key").value.length === 0) return;
  if(gel("cb-stm").checked && gel("stm_json_key").value.length === 0) return;
  if(gel("server_auth").value === "basic"){
    if(gel("client_username").value.length === 0) return;
    if(gel("client_password").value.length === 0) return;
  }
  if(gel("server_auth").value === "tls"){
    if(gel("server_ca").value.length === 0) return;
    if(gel("client_crt").value.length === 0) return;
    if(gel("client_key").value.length === 0) return;
  }
  gel("next").disabled = false;
}

/* Navigation api */
function handleNextClick() {
  switch(selectedForm){
    case "wifi_setup":
      selectedForm = "ipv4_setup";
      hide("wifi_setup");
      show("ipv4_setup");
      handleUseNtp(gel("cb-ntp").checked);
      handleIpv4MethodChange(gel("ipv4_method").value);
      break;
    case "ipv4_setup":
      selectedForm = "server_setup";
      hide("ipv4_setup");
      show("server_setup");
      gel("next").innerText = "Finish"
      handleUseEspKey(gel("cb-esp").checked);
      handleUseStmKey(gel("cb-stm").checked);
      handleServerAuthChange(gel("server_auth").value)
      break;
    case "server_setup":
      selectedForm = "ap_select";
      show("ap_select");
      hide("server_setup");
      break;
  }
}

function handleBackClick() {
  switch(selectedForm){
    case "wifi_setup":
      selectedForm = "ap_select";
      show("ap_select");
      hide("wifi_setup");
      hide("navigation");   
      break;
    case "ipv4_setup":
      selectedForm = "wifi_setup";
      show("wifi_setup");
      hide("ipv4_setup");
      break;
    case "server_setup":
      selectedForm = "ipv4_setup";
      hide("server_setup");
      show("ipv4_setup");
      gel("next").innerText = "Next"
      break;
  }
}

/* Refresh access points api */
var refreshAPInterval = null;
var checkStatusInterval = null;

function docReady(fn) {
  // see if DOM is already available
  if (
      document.readyState === "complete" ||
      document.readyState === "interactive"
      ) {
          // call on next available tick
          setTimeout(fn, 1);
  } else {
      document.addEventListener("DOMContentLoaded", fn);
  }
}

function startCheckStatusInterval() {
  checkStatusInterval = setInterval(checkStatus, 950);
}

function startRefreshAPInterval() {
  refreshAPInterval = setInterval(refreshAP, 3800);
}

function stopRefreshAPInterval() {
  if (refreshAPInterval != null) {
      clearInterval(refreshAPInterval);
      refreshAPInterval = null;
  }
}

function stopCheckStatusInterval() {
  if (checkStatusInterval != null) {
      clearInterval(checkStatusInterval);
      checkStatusInterval = null;
  }
}

docReady(async function () {

  gel("wifi-list").addEventListener(
    "click",
    (e) => {
      handleSelectSSID(e.target.innerText);
    },
    false
  );

  //first time the page loads: attempt get the connection status and start the wifi scan
  await refreshAP();
  startCheckStatusInterval();
  startRefreshAPInterval();
});

async function refreshAP(url = "ap.json") {
  try {
    var res = await fetch(url);
    var access_points = await res.json();
    if (access_points.length > 0) {
          //sort by signal strength
          access_points.sort((a, b) => {
            var x = a["rssi"];
            var y = b["rssi"];
            return x < y ? 1 : x > y ? -1 : 0;
          });
      refreshAPHTML(access_points);
    }
  } catch (e) {
    console.info("Access points returned empty from /ap.json!");
  }
}

function refreshAPHTML(data) {
  var h = "";
  data.forEach(function (e, idx, array) {
    let ap_class = idx === array.length - 1 ? "" : " brdb";
    let rssicon = rssiToIcon(e.rssi);
    let auth = e.auth == 0 ? "" : "pw";
    h += `<div class="ape${ap_class}"><div class="${rssicon}"><div class="${auth}">${e.ssid}</div></div></div>\n`;
  });

  gel("wifi-list").innerHTML = h;
}

function rssiToIcon(rssi) {
  if (rssi >= -60) {
    return "w0";
  } else if (rssi >= -67) {
    return "w1";
  } else if (rssi >= -75) {
    return "w2";
  } else {
    return "w3";
  }
}

async function checkStatus(url = "status.json") {
}