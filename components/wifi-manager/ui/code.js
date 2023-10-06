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
    if(gel("client_ca").value.length === 0) return;
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
      createSetup()
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

function startRefreshAPInterval() {
  refreshAPInterval = setInterval(refreshAP, 3800);
}

function stopRefreshAPInterval() {
  if (refreshAPInterval != null) {
      clearInterval(refreshAPInterval);
      refreshAPInterval = null;
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

  gel("ok-credits").addEventListener(
    "click",
    () => {
      hide("credits");
      show("ap_select");
    },
    false
  );

  gel("acredits").addEventListener(
    "click",
    () => {
      event.preventDefault();
      show("credits");
      hide("ap_select");
    },
    false
);

  //first time the page loads: attempt get the connection status and start the wifi scan
  await refreshAP();
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

async function createSetup() {
  hide("server_setup");
  hide("navigation");
  show("loading");

  const setup = await GetSetup();

  let count = Object.keys(setup).length;

  const timeout = () => { 
    if (count === 0) {
      connect();
    }else {
      hide("loading");
      show("setup-fail");
    }
  };

  let timer = setTimeout(timeout, 5000);

  const connect = () => {
    clearTimeout(timer);
    fetch("connect") 
    .then(function (res) {
      if(res.status === 200) {
        selectedForm = "ap_select";
        hide("loading");
        show("ap_select"); 
      }else {
        hide("loading");
        show("setup-fail"); 
      }
    })
    .catch (function (error) {
      console.log('Connect error: ', error);
      hide("loading");
      show("setup-fail");
    });
  };

  for (const [key, value] of Object.entries(setup)) {
      fetch(key, {
      method: "POST",
      headers: {
        'Content-Type': 'application/json;charset=utf-8'
      },
      body: value
    })
    .finally(() => {
      --count;
      if(count === 0) {
        connect();
      }
    })
  }
}

function GetHttpSetup() {
  return {
    server_address: gel("server_address").value,
    server_port: gel("server_port").value,
    server_api: gel("server_api").value,
    esp_json_key: gel("esp_json_key").value,
    stm_json_key: gel("stm_json_key").value,
    server_auth: gel("server_auth").value,
    client_username: gel("client_username").value,
    client_password: gel("client_password").value,
  }
}

function GetIpv4Setup() {
  return {
    ipv4_method: gel("ipv4_method").value,
    ipv4_address: gel("ipv4_address").value,
    ipv4_mask: gel("ipv4_mask").value,
    ipv4_gate: gel("ipv4_gate").value,
    ipv4_dns1: gel("ipv4_dns1").value,
    ipv4_dns2: gel("ipv4_dns2").value,
    ipv4_zone: gel("ipv4_zone").value,
    ipv4_ntp: gel("ipv4_ntp").value,
  }
}

function GetWifiSetup() {
  return {
    wifi_ssid: gel("wifi_ssid").value,
    wifi_wpa: gel("wifi_wpa").value,
    wifi_auth: gel("wifi_auth").value,
    wifi_identity: gel("wifi_identity").value,
    wifi_username: gel("wifi_username").value,
    wifi_password: gel("wifi_password").value,
  }
}

async function GetSetup() {
  let setup = {
    client_ca: "client_ca",
    client_crt: "client_crt",
    client_key: "client_key",
    wifi_ca: (gel("wifi_auth").value === "tls") ? "wifi_tls_ca" : "wifi_ca",
    wifi_crt: "wifi_crt",
    wifi_key: "wifi_key",
  }

  for (const [key, value] of Object.entries(setup)){
    const file = gel(value).files[0];
    setup[key] = JSON.stringify({[key]: (file) ? await file.text() : ''});
  }

  setup["wifi_setup"] = JSON.stringify(GetWifiSetup());
  setup["ipv4_setup"] = JSON.stringify(GetIpv4Setup());
  setup["http_setup"] = JSON.stringify(GetHttpSetup());

  return setup;
}

function handleGoBackClick() {
  hide("setup-fail");
  gel("navigation").style.display = "flex";
  selectedForm = "server_setup";
  show("server_setup");
  gel("next").innerText = "Finish"
  handleUseEspKey(gel("cb-esp").checked);
  handleUseStmKey(gel("cb-stm").checked);
  handleServerAuthChange(gel("server_auth").value)
}

function handleCancelClick() {
  selectedForm = "ap_select";
  gel("next").innerText = "Next"
  hide("setup-fail");
  show("ap_select");
}