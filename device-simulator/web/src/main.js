'use strict';

require('babel-core/register')();
const electron = require('electron');
const app = electron.app;
const BrowserWindow = electron.BrowserWindow;
require('electron-reload')(__dirname);

var mainWindow;

function createWindow () {
  mainWindow = new BrowserWindow({
    icon: __dirname + '/images/app_icon.png'
  });
  mainWindow.loadURL('file://' + __dirname + '/index.html');
  mainWindow.maximize();
  mainWindow.on('closed', function() {
    mainWindow = null;
  });
}

app.on('ready', createWindow);

app.on('window-all-closed', function () {
  if (process.platform !== 'darwin') {
    app.quit();
  }
});

app.on('activate', function () {
  if (mainWindow === null) {
    createWindow();
  }
});
