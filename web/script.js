
var uploadForm     = document.getElementById('upload-form');
var deleteSelected = document.getElementById('delete-selected');
var inputField     = document.getElementById('inputField');
var output         = document.getElementById('output');
var autoScroll     = document.getElementById('autoScroll');
var cmdHist        = [];
var cmdHistIndex   = 0;

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

uploadForm.addEventListener('submit', function(event) {
  event.preventDefault();

  var formData = new FormData(uploadForm);
  fetch('/upload', {
    method: 'POST',
    body: formData
  })
  .then(response => {
      if(response.ok) {
          fetchAndDisplayFiles(); // Refresh the file list
      } else {
          alert('Upload failed');
      }
  })
  .catch(error => console.error('Error:', error));
});

deleteSelected.addEventListener('click', () => {
    const selectedFiles = Array.from(document.querySelectorAll('.file-checkbox:checked'))
                              .map(checkbox => checkbox.dataset.filename);
    if (selectedFiles.length > 0 && confirm('Are you sure you want to delete the selected files?')) {
        selectedFiles.forEach(filename => {
            fetch('/delete-file', {
                method: 'POST',
                headers: { 'Content-Type': 'text/plain' },
                body: filename
            }).then(() => fetchAndDisplayFiles()); // Refresh the list
        });
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

function fetchAndDisplayFiles() {
    fetch('/list-files')
        .then(response => response.json())
        .then(files => {
            const tableBody = document.querySelector('#files-table tbody');
            tableBody.innerHTML = ''; // Clear existing rows
            files.forEach(file => {
                const row = document.createElement('tr');
                row.innerHTML = `
                    <td><input type="checkbox" class="file-checkbox" data-filename="${file.name}"></td>
                    <td><a href="/download?filename=${encodeURIComponent(file.name)}">${file.name}</a></td>
                    <td>${file.size} bytes</td>
                `;
                tableBody.appendChild(row);
            });
        });
}

function downloadFile(filename) {
    window.open('/download?filename=' + encodeURIComponent(filename), '_blank');
}

document.addEventListener('DOMContentLoaded', fetchAndDisplayFiles);

setInterval(poll, 2000); // Poll every 2 seconds