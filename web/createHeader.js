const fs = require('fs');
const path = require('path');

function escapeForCPlusPlus(str) {
  return str.replace(/\\/g, '\\\\')
            .replace(/"/g, '\\"')
            .replace(/\n/g, '\\n');
}

function readFileContents(filePath) {
  return new Promise((resolve, reject) => {
    fs.readFile(filePath, 'utf8', (err, data) => {
      if (err) reject(err);
      else resolve(data);
    });
  });
}

async function createHeaderFile() {
  try {
    const htmlContent = await readFileContents('index.min.html');
    const cssContent = await readFileContents('style.min.css');
    const jsContent = await readFileContents('script.min.js');

    let headerContent = '#ifndef WEB_CONTENT_H\n#define WEB_CONTENT_H\n\n';

    headerContent += `const char* html_content = "${escapeForCPlusPlus(htmlContent)}";\n\n`;
    headerContent += `const char* css_content = "${escapeForCPlusPlus(cssContent)}";\n\n`;
    headerContent += `const char* js_content = "${escapeForCPlusPlus(jsContent)}";\n\n`;

    headerContent += '#endif // WEB_CONTENT_H\n';

    fs.writeFileSync('../web-content.h', headerContent);
    console.log('Header file created successfully');
  } catch (err) {
    console.error('Error creating header file:', err);
  }
}

createHeaderFile();
