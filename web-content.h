#ifndef WEB_CONTENT_H
#define WEB_CONTENT_H

const char* html_content = "<!DOCTYPE html><html lang=\"en\"><head><meta charset=\"UTF-8\"><title>Microcontroller Interface</title><link rel=\"stylesheet\" href=\"style.min.css\"></head><body><div class=\"tab-header\"><button class=\"tab-button active\" onclick='openTab(\"serial\")'>Serial Interface</button> <button class=\"tab-button\" onclick='openTab(\"upload\")'>File Upload</button> <button class=\"tab-button\" onclick='openTab(\"settings\")'>Settings</button></div><div id=\"serial\" class=\"tab active\"><input type=\"text\" id=\"inputField\" placeholder=\"Type here...\"> <button onclick=\"submitData()\">Submit</button> <button onclick=\"clearOutput()\">Clear Output</button> <label><input type=\"checkbox\" id=\"autoScroll\" checked=\"checked\"> Scroll to bottom</label><div id=\"output\"></div></div><div id=\"upload\" class=\"tab\"><form action=\"/upload\" method=\"post\" enctype=\"multipart/form-data\"><input type=\"file\" name=\"file\"> <input type=\"submit\" value=\"Upload File\"></form></div><div id=\"settings\" class=\"tab\"><div style=\"display:grid;grid-template-columns:auto auto;gap:10px\"><div>Setting 1</div><input type=\"text\"></div></div><script src=\"script.min.js\"></script></body></html>";

const char* css_content = "body{font-family:Arial,sans-serif;margin:0;padding:20px;box-sizing:border-box}.tab{display:none}.tab.active{display:block}.tab-header{margin-bottom:20px}.tab-button{padding:10px;cursor:pointer}.tab-button.active{font-weight:700}#inputField{width:calc(100% - 10px);box-sizing:border-box}#output{height:calc(100vh - 220px);overflow-y:scroll;background:#f0f0f0;margin-top:10px;padding:5px;font-family:'Courier New',monospace;white-space:pre}button{margin-top:5px}";

const char* js_content = "var uploadForm=document.querySelector(\"#upload form\"),inputField=document.getElementById(\"inputField\"),output=document.getElementById(\"output\"),autoScroll=document.getElementById(\"autoScroll\"),cmdHist=[],cmdHistIndex=0;function openTab(t){for(var e,n=document.getElementsByClassName(\"tab\"),o=0;o<n.length;o++)n[o].style.display=\"none\";for(e=document.getElementsByClassName(\"tab-button\"),o=0;o<e.length;o++)e[o].className=e[o].className.replace(\" active\",\"\");document.getElementById(t).style.display=\"block\",event.currentTarget.className+=\" active\"}function submitData(){var t=inputField.value;cmdHist.push(t),cmdHistIndex=cmdHist.length,fetch(\"/submit\",{method:\"POST\",body:t}),inputField.value=\"\"}function clearOutput(){output.innerHTML=\"\"}function poll(){fetch(\"/poll\").then(t=>t.json()).then(t=>{var e=autoScroll.checked&&output.scrollTop+output.clientHeight===output.scrollHeight;t.forEach(t=>{var e=document.createElement(\"div\");e.textContent=\"\"!=t?t:\" \",output.appendChild(e)}),e&&(output.scrollTop=output.scrollHeight)})}inputField.addEventListener(\"keydown\",function(t){\"Enter\"===t.key?submitData():\"ArrowUp\"===t.key?(cmdHistIndex=Math.max(cmdHistIndex-1,0),inputField.value=cmdHist[cmdHistIndex]||\"\"):\"ArrowDown\"===t.key&&(cmdHistIndex=Math.min(cmdHistIndex+1,cmdHist.length-1),inputField.value=cmdHist[cmdHistIndex]||\"\")}),uploadForm.addEventListener(\"submit\",function(t){t.preventDefault();t=new FormData(uploadForm);fetch(\"/upload\",{method:\"POST\",body:t}).then(t=>t.text()).then(t=>console.log(t)).catch(t=>console.error(\"Error:\",t))}),setInterval(poll,2e3);";

#endif // WEB_CONTENT_H