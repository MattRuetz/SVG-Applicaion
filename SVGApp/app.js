'use strict'

// C library API
const ffi = require('ffi-napi');

// Express App (Routes)
const express = require("express");
const app     = express();
const path    = require("path");
const fileUpload = require('express-fileupload');

app.use(fileUpload());
app.use(express.static(path.join(__dirname+'/uploads')));

// Minimization
const fs = require('fs');
const JavaScriptObfuscator = require('javascript-obfuscator');

// Important, pass in port as in `npm run dev 1234`, do not change
const portNum = process.argv[2];

// Send HTML at root, do not change
app.get('/',function(req,res){
  res.sendFile(path.join(__dirname+'/public/index.html'));
});

// Send Style, do not change
app.get('/style.css',function(req,res){
  res.sendFile(path.join(__dirname+'/public/style.css'));
});

// Send obfuscated JS, do not change
app.get('/index.js',function(req,res){
  fs.readFile(path.join(__dirname+'/public/index.js'), 'utf8', function(err, contents) {
    const minimizedContents = JavaScriptObfuscator.obfuscate(contents, {compact: true, controlFlowFlattening: true});
    res.contentType('application/javascript');
    res.send(minimizedContents._obfuscatedCode);
  });
});

//Respond to POST requests that upload files to uploads/ directory
app.post('/upload', function(req, res) {
  if(!req.files) {
    return res.status(400).send('No files were uploaded.');
  }

  console.log(req.files);

  let uploadFile = req.files.uploadFile;

  
  // Use the mv() method to place the file somewhere on your server
  uploadFile.mv('uploads/' + uploadFile.name, function(err) {
    if(err) {
      return res.status(500).send(err);
    }
  });

  res.status(200).send("Success!");


});

//Respond to GET requests for files in the uploads/ directory
app.get('/uploads/:name', function(req , res){
  fs.stat('uploads/' + req.params.name, function(err, stat) {
    if(err == null) {
      res.sendFile(path.join(__dirname+'/uploads/' + req.params.name));
    } else {
      console.log('Error in file downloading route: '+err);
      res.send('');
    }
  });
});

let sharedLib = ffi.Library('./libsvgparse.so', {

  'svgFileToJSON': ['string', ['string', 'string']],
  'svgFileToComponentListJSON': ['string', ['string', 'string']],
  'svgFileToAttrListJSON': ['string', ['string', 'string', 'string']],
  'svgFileToTitleAndDescJSON': ['string',['string','string']],
  'changeTitle': ['int',['string','string','string']],
  'changeDesc': ['int',['string','string','string']],
  'editAttrFromSVGFile': ['int',['string','string','string','string','string']],
  'createSVGFromScratch': ['int', ['string','string','string']],
  'addRectToSVG': ['int', ['string','string','string']],
  'addCircToSVG': ['int', ['string','string','string']],
  'scaleRects': ['void', ['string','string','float']],
  'scaleCircs': ['void', ['string','string','float']]

});



app.get('/getSVGJSONs', function(req , res){

  let dirPath = req.query.dirPath;

  let svgObjArr = [];

  fs.readdir(dirPath, function(err, items){
    console.log(items);

    for(var i = 0; i<items.length; ++i){
      console.log("Adding svg file: "+items[i]);

      let retStr = sharedLib.svgFileToJSON("./"+req.query.dirPath + items[i],"svg.xsd");

      let svgObj = JSON.parse(retStr);

      svgObjArr.push(svgObj);

    }

    res.send({
      svgJSONs: svgObjArr
    });
  });
});


app.get('/getSVGFileInfo', function(req , res){

  let dirPath = req.query.dirPath;

  let svgFileNameArr = [];
  let svgFileSizeArr = [];

  fs.readdir(dirPath, function(err, items){
    if(err == null){

      console.log(items);

      for(var i = 0; i<items.length; ++i){

        console.log(req.query.dirPath + items[i]);

        var stats = fs.statSync(req.query.dirPath + items[i]);

        svgFileNameArr.push(items[i]);

        let fileSize_kB = stats["size"] / 1000.0;

        console.log(fileSize_kB);

        svgFileSizeArr.push(fileSize_kB);
      }
      
      res.send({nameArr: svgFileNameArr, sizeArr: svgFileSizeArr});
    }else{
      return res.status(500).send(err);
    }
  });
});


app.get('/getSVGComponents', function(req, res){

  let filePath = req.query.filePath;

  let svgComponentArr = [[]];

  //Call C function to get JSON string of all svg components
  let retStr = sharedLib.svgFileToComponentListJSON(filePath, "svg.xsd");

  try{
    svgComponentArr = JSON.parse(retStr);
  }
  catch(err){
    return res.status(500).send(err);
  }
  console.log("Displaying components: "+svgComponentArr);

  res.send({
    svgComponentArr: svgComponentArr
  });

});

app.get('/getTitleDesc', function(req, res){

  let filePath = req.query.filePath;


  let retStr = sharedLib.svgFileToTitleAndDescJSON(filePath, "svg.xsd");

  try{
    let titleDescObj = JSON.parse(retStr);

    if(titleDescObj['title']=='')titleDescObj['title']='N/A';
    if(titleDescObj['desc']=='')titleDescObj['desc']='N/A';
    res.send({
      titleDesc: titleDescObj
    });

  }catch(err){
    res.status(500);
    res.send({
      error: err
    });
  }
  
});


app.get('/getCompOtherAttributes', function(req, res){

  let filePath = req.query.filePath;
  let compDesc = req.query.compName;

  console.log("opening file: "+filePath);
  let retStr = sharedLib.svgFileToAttrListJSON(filePath, "svg.xsd", compDesc);

  try{
    let otherAttrObj = JSON.parse(retStr);
    res.send({
      otherAttr: otherAttrObj
    });
  }catch(err){
    res.status(500);
    res.send({
      error: err
    });
  }

});


app.get('/updateTitle', function(req, res){

  try{

    sharedLib.changeTitle(req.query.filePath, "svg.xsd", req.query.newTitle);

  }catch(err){
    res.status(500);
    res.send({
      error: err
    });
  }

});

app.get('/updateDesc', function(req, res){

  try{

    sharedLib.changeDesc(req.query.filePath, "svg.xsd", req.query.newDesc);

  }catch(err){
    res.status(500);
    res.send({
      error: err
    });
  }

});


app.get('/editAttr', function(req, res){

  try{
    console.log(req.query);
    sharedLib.editAttrFromSVGFile(req.query.filePath, "svg.xsd", req.query.compName, req.query.attrName, req.query.attrVal);

  }catch(err){
    res.status(500);
    res.send({
      error: err
    });
  }

});


app.get('/createNewSVG', function(req, res){

  try{

    console.log("Saving: "+req.query.svgInfo);
    sharedLib.createSVGFromScratch(req.query.filePath, "svg.xsd", req.query.svgInfo);

  }catch(err){
    res.status(500);
    res.send({
      error: err
    });
  }
  
});

app.get('/addRect', function(req, res){


  console.log("adding: "+req.query.rectJSON);

  try{
    let rectNum = sharedLib.addRectToSVG(req.query.filePath, "svg.xsd", req.query.rectJSON);

    console.log("File saved: "+req.query.filePath);
    res.send({
        rectNum: rectNum
    });

  }catch(err){
    res.status(500);
    res.send({
      error: err
    });
  }
  
});


app.get('/addCirc', function(req, res){


  console.log("adding: "+req.query.circJSON);

  try{
    let circNum = sharedLib.addCircToSVG(req.query.filePath, "svg.xsd", req.query.circJSON);

    console.log("File saved: "+req.query.filePath);
    res.send({
        circNum: circNum
    });

  }catch(err){
    res.status(500);
    res.send({
      error: err
    });
  }
  
});


app.get('/scaleRects', function(req, res){


  console.log("Scaling all rectangles in: "+req.query.filePath +" by x"+req.query.scaleFactor);

  try{
    sharedLib.scaleRects(req.query.filePath, "svg.xsd", req.query.scaleFactor);

    console.log("File saved: "+req.query.filePath);

  }catch(err){
    res.status(500);
    res.send({
      error: err
    });
  }
  
});

app.get('/scaleCircs', function(req, res){


  console.log("Scaling all circles in: "+req.query.filePath +" by x"+req.query.scaleFactor);

  try{
    sharedLib.scaleCircs(req.query.filePath, "svg.xsd", req.query.scaleFactor);

    console.log("File saved: "+req.query.filePath);

  }catch(err){
    res.status(500);
    res.send({
      error: err
    });
  }
  
});



app.listen(portNum);
console.log('Running app at localhost: ' + portNum);
