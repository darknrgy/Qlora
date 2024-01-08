
  var inputField   = document.getElementById('inputField');
  var output       = document.getElementById('output');
  var autoScroll   = document.getElementById('autoScroll');
  var cmdHist      = [];
  var cmdHistIndex = 0;

  inputField.addEventListener('keydown', function(e) {
    if (e.key === 'Enter') {
      submitData();
    } else if (e.key === 'ArrowUp') {
      cmdHistIndex = Math.max(cmdHistIndex - 1, 0);
      inputField.value = cmdHist[cmdHistIndex] || '';
    } else if (e.key === 'ArrowDown') {
      cmdHistIndex = Math.min(cmdHistIndex + 1, cmdHist.length - 1);
      inputField.value = cmdHist[cmdHistIndex] || '';
    }
  });

  function openTab(tabName) {
    var i, tabcontent, tablinks;
    tabcontent = document.getElementsByClassName("tab");
    for (i = 0; i < tabcontent.length; i++) {
      tabcontent[i].style.display = "none";
    }
    tablinks = document.getElementsByClassName("tab-button");
    for (i = 0; i < tablinks.length; i++) {
      tablinks[i].className = tablinks[i].className.replace(" active", "");
    }
    document.getElementById(tabName).style.display = "block";
    event.currentTarget.className += " active";
  }

  function submitData() {
    var data = inputField.value;
    cmdHist.push(data);
    cmdHistIndex = cmdHist.length;
    fetch('/submit', { method: 'POST', body: data });
    inputField.value = '';
  }

  function clearOutput() {
    output.innerHTML = '';
  }

  function poll() {
    fetch('/poll')
      .then(response => response.json())
      .then(data => {
        var shouldScroll = autoScroll.checked && output.scrollTop + output.clientHeight === output.scrollHeight;
        
        data.forEach(line => {
          var div = document.createElement('div');
          // Make sure we have at least a non-breaking space on empty lines
          div.textContent = line != "" ? line : " ";
          output.appendChild(div);
        });

        if (shouldScroll) {
          output.scrollTop = output.scrollHeight;
        }
      });
  }

  setInterval(poll, 2000); // Poll every 2 seconds