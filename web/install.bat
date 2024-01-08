@echo off
echo "Installing Dependencies..."
call npm install -g html-minifier
call npm install -g clean-css-cli
call npm install -g uglify-js
echo "Done."