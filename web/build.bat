@echo off

del style.min.css
del script.min.js
del index.min.html

call cleancss -o style.min.css style.css
call uglifyjs script.js -o script.min.js -c -m
call html-minifier --collapse-whitespace --remove-comments --minify-css true --minify-js true index.html -o index.min.html

call node createHeader.js