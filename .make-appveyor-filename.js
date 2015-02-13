var util = require('util');
var sanitize = require("sanitize-filename");

var dstFileName = process.env.APPVEYOR_REPO_COMMIT.substring(0, 8) + " - " + 
                process.env.APPVEYOR_REPO_COMMIT_MESSAGE.substring(0, 100) + ".7z";
console.log(sanitize(dstFileName));

