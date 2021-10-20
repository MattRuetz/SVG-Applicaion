// Put all onload AJAX calls here, and event listeners
$(document).ready(function() {
    // On page-load AJAX Example
    $.ajax({
        type: 'get',            //Request type
        dataType: 'json',       //Data type - we will use JSON for almost everything 
        url: '/getSVGJSONs',   //The server endpoint we are connecting to
        data: {
            dirPath: "uploads/",
        },
        success: function (data) {
              /*Here, the SVG table at the top of the page
              is created with the correct number of cells to
              show info for all SVGs on server. The server populates
              the last 4 columns of the table using the C shared library */

            let table = document.getElementById("svgList-table");

            console.log("svgJSON List from server: "+data.svgJSONs);

            for(var i=0; i< data.svgJSONs.length; ++i){

                let row = table.insertRow()

                let cell1 = row.insertCell(0); //actual image thumbnail
                let cell2 = row.insertCell(1); //file name
                let cell3 = row.insertCell(2);
                let cell4 = row.insertCell(3);
                let cell5 = row.insertCell(4);
                let cell6 = row.insertCell(5);
                let cell7 = row.insertCell(6);

                cell4.innerHTML = data.svgJSONs[i]['numRect'];
                cell5.innerHTML = data.svgJSONs[i]['numCirc'];
                cell6.innerHTML = data.svgJSONs[i]['numPaths'];
                cell7.innerHTML = data.svgJSONs[i]['numGroups'];

            }

            console.log(data); 

        },
        error: function(error) {                                               //ADD ERROR FUNC
            // Non-200 return, do something with error
            alert("On page load, received error from server");
            console.log(error); 
        }
    });

//--------------------------------------------------------------------

    $.ajax({
        type: 'get',            //Request type
        dataType: 'json',       //Data type - we will use JSON for almost everything 
        url: '/getSVGFileInfo',   //The server endpoint we are connecting to
        data: {
            dirPath: "uploads/",
        },
        success: function (data) {

            /*This Ajax gets names and file sizes of the server's SVGs
            and places them in the proper cells of the previously made table*/

            let table = document.getElementById("svgList-table");
            let dd = document.getElementById("svg-dropdown");
            let dd2 = document.getElementById("edit-dropdown");
            let dd3 = document.getElementById("add-dropdown");
            let dd4 = document.getElementById("scale-dropdown");

            console.log("names: "+data.nameArr + " sizes: "+data.sizeArr);

            for(var i=0; i< data.nameArr.length; ++i){

                let svgFile = data.nameArr[i];

                var row = table.rows[i+1];

                let cell1 = row.cells[0];
                let cell2 = row.cells[1];
                let cell3 = row.cells[2];

                //Add image for each file as DL link
                var img = document.createElement("img");
                img.className = "thumbnail";
                img.src = svgFile;

                var a1 = document.createElement("a");
                a1.download = svgFile;
                a1.href = svgFile;
                a1.title = svgFile;

                a1.appendChild(img); //add image as child of hyperlink
                cell1.appendChild(a1); //link as child of cell


                //Add in file name as download link to cell2 for each svg
                var a2 = document.createElement("a");
                a2.download = svgFile;
                a2.href = svgFile;
                a2.title = svgFile;
                a2.innerHTML = svgFile;

                cell2.align = "center";
                cell2.appendChild(a2); //link as child of cell

                cell3.innerHTML = data.sizeArr[i] + "kB";

                //This section populates the svg dropdown menus

                let a_dd = document.createElement("a");
                let a_dd2 = document.createElement("a");
                let a_dd3 = document.createElement("a");
                let a_dd4 = document.createElement("a");

                a_dd.innerHTML = svgFile;
                a_dd2.innerHTML = svgFile;
                a_dd3.innerHTML = svgFile;
                a_dd4.innerHTML = svgFile;

                a_dd.addEventListener("click", function RespondClick(){
                    console.log("clicked dropdown item: "+a_dd.innerHTML);

                    setImgTable(a_dd.innerHTML);
                });

                a_dd2.addEventListener("click", function RespondClick(){

                    let btn = document.getElementById("edit-dropdown-btn");
                    btn.innerHTML = a_dd2.innerHTML+" ▼";
                    console.log("clicked dropdown item: "+a_dd2.innerHTML);

                    //CALL FUNCTION to set EDIT an attribute or title/desc
                    setEditSVGForm(svgFile);

                    fillEditAttrCompList(svgFile);

                });

                a_dd3.addEventListener("click", function RespondClick(){
                    console.log("clicked dropdown item: "+a_dd2.innerHTML);


                    let a_add = document.getElementById("add-label-link");
                    a_add.innerHTML = svgFile;

                    //CALL FUNCTION to set ADD a shape
                });

                a_dd4.addEventListener("click", function RespondClick(){
                    console.log("clicked dropdown item: "+a_dd4.innerHTML);


                    let a_add2 = document.getElementById("scale-label");
                    a_add2.innerHTML = svgFile;

                    //CALL FUNCTION to set ADD a shape
                });


                dd.appendChild(a_dd);
                dd2.appendChild(a_dd2);
                dd3.appendChild(a_dd3);
                dd4.appendChild(a_dd4);
            }

            console.log(data); 

        },
        error: function(error) {
            // Non-200 return, do something with error
            alert("On page load, received error from server");
            console.log(error); 
        }
    });

//--------------------------------------------------------------------

    /* This function fills in the SVG view panel with all information
     about the selected SVG. It is called by a Event Listener which
     is activated when a dropdown item is selected to view.
     */
    function setImgTable(svgFile){

        console.log("Filling table for svg: "+ svgFile);

        $.ajax({
            type: "get",
            dataType: "json",
            url: "/getSVGComponents",
            data: {
                filePath: "./uploads/"+svgFile,
            },
            success: function(data){

                let filePath = "./uploads/"+svgFile;
                let link = document.getElementById("svg-view-link");
                let dispTable = document.getElementById("disp-img-table");
                //let dispImg = document.getElementById("disp-img");
                let comp_dd = document.getElementById("component-dropdown");

                comp_dd.innerHTML = ''
                link.innerHTML = ''
                dispTable.innerHTML = ''

                var table = document.createElement("table");
                table.id = "img-table";
                row1 = table.insertRow();

                dispImg = row1.insertCell(0);
                dispImg.align = "center";
                dispImg.colSpan = "3";

                row2 = table.insertRow();
                row2.className = "heading-row";

                titCell = row2.insertCell(0);
                titCell.innerHTML = "Title"

                descCell = row2.insertCell(1);
                descCell.colSpan = "2";
                descCell.innerHTML = "Description";

                row3 = table.insertRow();
                row3.className = "heading-row";

                compCell = row3.insertCell(0);
                compCell.innerHTML = "Component";

                summaryCell = row3.insertCell(1);
                summaryCell.innerHTML = "Summary";

                oACell = row3.insertCell(2);
                oACell.innerHTML = "Other Attributes";

                dispTable.appendChild(table);

                var a = document.createElement("a");
                a.download = svgFile;
                a.href = svgFile;
                a.title = svgFile;
                a.innerHTML = svgFile;
                link.appendChild(a);

                var img = document.createElement("img");
                img.className = "svg-display";
                img.src = filePath;
                dispImg.innerHTML = '';
                dispImg.appendChild(img);

                

                for(var i=0; i<4; ++i){
                    //data.svgComponentArr[i];

                    var compName;
                    if(i==0){
                        compName = "Rectangle ";
                    }else if(i==1){
                        compName = "Circle ";
                    }else if(i==2){
                        compName = "Path ";
                    }else if(i==3){
                        compName = "Group ";
                    }

                    for (var j=0; j<data.svgComponentArr[i].length;++j){

                        var row = table.insertRow();

                        var cell1 = row.insertCell(0); //component name
                        var cell2 = row.insertCell(1); //summary
                        var cell3 = row.insertCell(2);

                        let curShapeInd = (j+1);

                        cell1.innerHTML = compName+(curShapeInd);

                        let new_comp = document.createElement("a");
                        new_comp.innerHTML = compName+(curShapeInd);

                        new_comp.addEventListener("click", function RespondClick(){
                            console.log("clicked dropdown item: "+new_comp.innerHTML);

                            setOtherAttrTable(svgFile, new_comp.innerHTML);
                        });

                        comp_dd.appendChild(new_comp);

                        //cell2.innerHTML = " | ";
                        for(var itemName in data.svgComponentArr[i][j]){
                            itemVal = data.svgComponentArr[i][j][itemName];

                            cell2.innerHTML += itemName + ": " +itemVal + "<br>";

                        }
                        fillOtherAttrCount(svgFile, cell1.innerHTML, cell3);
                    }
                }

            },
            error: function(error){
                alert("Error disiplaying image information!");
                console.log(error); 
            }

        });

        function fillOtherAttrCount(svgFile, compName, cell){

            $.ajax({
                type: "get",
                dataType: "json",
                url: "/getCompOtherAttributes",
                data: {
                    filePath: "./uploads/"+svgFile,
                    compName: compName,
                },
                success: function(data){

                    var attrCount = data.otherAttr.length;
                    cell.innerHTML = attrCount;
                },
                error: function(error){
                    alert("Error counting other attributes!");
                    console.log(error); 
                }
            });
        }

        $.ajax({

            type: 'get',            //Request type
            dataType: 'json',       //Data type - we will use JSON for almost everything 
            url: '/getTitleDesc',   //The server endpoint we are connecting to
            data: {
                filePath: "uploads/"+svgFile,
            },
            success: function (data) {
                let table = document.getElementById("img-table");

                var row = table.insertRow(2);
                let titCell = row.insertCell(0);
                let descCell = row.insertCell(1);
                descCell.colSpan = "2";

                for(var param in data.titleDesc){

                    if(param == "title"){
                        titCell.innerHTML = data.titleDesc[param];
                    }
                    else{
                        descCell.innerHTML = data.titleDesc[param];
                    }
                }
            },
            error: function(error) {                                               //ADD ERROR FUNC
                // Non-200 return, do something with error
                window.alert("Error loading Title and Description for this SVG image!");
                console.log(error);
            }
    	});
    }

    /*This function gets all other attributes for a shape, and displays
     all of them in a table which is fully built dynamically
     */
    function setOtherAttrTable(svgFile, compName){

        console.log("Showing other attributes for: "+compName);

        let btn = document.getElementById("show-other-attr-btn");
        btn.innerHTML = compName + " ▼";

        $.ajax({
        type: "get",
        dataType: "json",
        url: "/getCompOtherAttributes",
        data: {
            filePath: "./uploads/"+svgFile,
            compName: compName,
        },
        success: function(data){

            let attrTable = document.getElementById("show-other-attr");
            attrTable.innerHTML = ''; //clear any existing table
            let tableHeading = document.createElement("h5");

            tableHeading.innerHTML =
             "Additional Attributes For:  <strong>"+compName+"</strong>"

            let table = document.createElement("table");

            headingRow = table.insertRow();

            headingRow.className = "heading-row";

            headingCell1 = headingRow.insertCell(0);
            headingCell1.className = "table-half-cell"
            headingCell1.innerHTML = "Attribute Name"
            headingCell2 = headingRow.insertCell(1);
            headingCell2.innerHTML = "Attribute Value"
            console.log(data.otherAttr);

            for(var i = 0; i<data.otherAttr.length; ++i){
                    attrName = data.otherAttr[i]["name"];
                    attrVal = data.otherAttr[i]["value"];

                    console.log("Adding: "+attrName +" = "+attrVal);
                    let row = table.insertRow();

                    let thisNameCell = row.insertCell(0);
                    thisNameCell.innerHTML = attrName;
                    thisNameCell.className = "table-half-cell";

                    let thisValCell = row.insertCell(1);
                    thisValCell.innerHTML = attrVal;
            }
            attrTable.appendChild(tableHeading);
            if(data.otherAttr.length == 0){

                let noneRow = table.insertRow();
                let noneCell = noneRow.insertCell();
                noneCell.colSpan = "2";
                noneCell.align = "center";
                noneCell.innerHTML = "<em>~This shape has no additional attributes~</em>"
            }
            attrTable.appendChild(table);

        },
        error: function(error){
            window.alert("Error displaying other attributes!");
            console.log(error); 
        }

        });
    }
    /*This function fills in the Title and Description box for the
    "Edit SVG Image" panel.
    */
    function setEditSVGForm(svgFile){

        $.ajax({

            type: 'get',            //Request type
            dataType: 'json',       //Data type - we will use JSON for almost everything 
            url: '/getTitleDesc',   //The server endpoint we are connecting to
            data: {
                filePath: "uploads/"+svgFile,
            },
            success: function (data) {
                let titleBox = document.getElementById("titleEntryBox");
                let descBox = document.getElementById("descEntryBox");
                let editLabel = document.getElementById("edit-label");
                let editAttr_dd = document.getElementById("component-attr-dropdown");
                
                editLabel.innerHTML = svgFile;

                for(var param in data.titleDesc){

                    if(param == "title"){
                        titleBox.value = data.titleDesc[param];
                    }
                    else{
                        descBox.value = data.titleDesc[param];
                    }
                }
            },
            error: function(error) {                                               //ADD ERROR FUNC
                // Non-200 return, do something with error
                window.alert("Error loading Title and Description for this SVG image!");
                console.log(error);
            }
        });
    }

    function fillEditAttrCompList(svgFile){

        $.ajax({
            type: "get",
            dataType: "json",
            url: "/getSVGComponents",
            data: {
                filePath: "./uploads/"+svgFile,
            },
            success: function(data){

                let filePath = "./uploads/"+svgFile;
                let link = document.getElementById("svg-view-link");
                let comp_dd = document.getElementById("component-attr-dropdown");
                let attrLabel = document.getElementById("edit-attr-label");

                attrLabel.innerHTML = '';
                comp_dd.innerHTML = '';
                link.innerHTML = '';

                for(var i=0; i<4; ++i){
                    var compName;
                    if(i==0){
                        compName = "Rectangle";
                    }else if(i==1){
                        compName = "Circle";
                    }else if(i==2){
                        compName = "Path";
                    }else if(i==3){
                        compName = "Group";
                    }

                    for (var j=0; j<data.svgComponentArr[i].length;++j){

                        let curShapeInd = (j+1);

                        let new_comp = document.createElement("a");
                        new_comp.innerHTML = compName+" " +(curShapeInd);

                        new_comp.addEventListener("click", function RespondClick(){
                            console.log("clicked dropdown item: "+new_comp.innerHTML);
                            attrLabel.innerHTML = new_comp.innerHTML;
                            
                        });

                        comp_dd.appendChild(new_comp);
                    }
                }

            },
            error: function(error){
                alert("Error disiplaying getting components for this image!");
                console.log(error); 
            }
        });//ASDMAOSMDKASD
    }


    $('#edit-title-form').submit(function(e){
        console.log("Form has data: "+$('#titleEntryBox').val());
        e.preventDefault();


        let titleBox = document.getElementById("titleEntryBox");
        var newTitle = titleBox.value;

        let editLabel = document.getElementById("edit-label");
        var svgFile = editLabel.innerHTML;


        if(newTitle.length > 256){
            window.alert("Invalid entry! Title must be 256 characters or less.");
        }
        else if(svgFile.innerHTML == "Choose image to edit ▼"){
            window.alert("Invalid selection! Please select a file from the dropdown menu.");
        }
        else{
            console.log("Title is valid. Editing svg...");

            $.ajax({

                type: 'get',
           		dataType: 'json',
            	url: '/updateTitle',
            	data: {
                    filePath: "./uploads/"+svgFile,
            	    newTitle: newTitle,
            	},
            	success: function (data) {

                    //Call functions to set the tables which include svg title
                    setEditSVGForm(svgFile);
                    setImgTable(svgFile);
            	},
            	error: function (error) {
                    window.alert("Error changing the title!");
                    console.log(error);
            	}
            });
        }
    });

    $('#edit-desc-form').submit(function(e){
        console.log("Form has data: "+$('#descEntryBox').val());
        e.preventDefault();

        let descBox = document.getElementById("descEntryBox");
        var newDesc = descBox.value;

        let editLabel = document.getElementById("edit-label");
        var svgFile = editLabel.innerHTML;


        if(newDesc.length > 256){
            window.alert("Invalid entry! Description must be 256 characters or less.");
        }
        else if(svgFile.innerHTML == "Choose image to edit ▼"){
            window.alert("Invalid selection! Please select a file from the dropdown menu.");
        }
        else{
            console.log("Description is valid. Editing svg...");

        //Pass data to the Ajax call, so it gets passed to the server
            $.ajax({
                type: 'get',
           		dataType: 'json',
            	url: '/updateDesc',
            	data: {
            	   filePath: "./uploads/"+svgFile,
                    newDesc: newDesc,
            	},
            	success: function (data) {

                    //if(res.retVal == 0){
                        //window.alert("ERROR: There was a problem saving the editted SVG file!");
                   //}
                    //Call functions to set the tables which include svg desc
                    setEditSVGForm(svgFile);
                    setImgTable(svgFile);
        
            	},
            	error: function (error) {
                    window.alert("Error changing the description!");
                    console.log(error);
            	}
            });
        }
    });

    $('#edit-attr-form').submit(function(e){
        console.log("Form has attrName: "+$('#attrNameEntryBox').val());
        console.log("Form has attrVal: "+$('#attrValEntryBox').val());
        e.preventDefault();

        let attrNameBox = document.getElementById("attrNameEntryBox");
        var attrName = attrNameBox.value;

        let attrValBox = document.getElementById("attrValEntryBox");
        var attrVal = attrValBox.value;

        let editAttrLabel = document.getElementById("edit-attr-label");
        var compName = editAttrLabel.innerHTML;

        let editLabel = document.getElementById("edit-label");
        var svgFile = editLabel.innerHTML;

        if(attrName.length > 512 || attrVal.length >512){
            window.alert("Invalid entry! Name or description is too long!");
        }
        else if(compName == ""){
            window.alert("Invalid selection! Please select a file from the dropdown menu.");
        }
        else{
            console.log("Attribute is valid. Editing svg...");

            $.ajax({
                type: 'get',
           		dataType: 'json',
            	url: '/editAttr',
            	data: {
            	   attrName: attrName,
                   attrVal: attrVal,
                   compName: compName,
                   filePath: "./uploads/"+svgFile,
            	},
            	success: function (data) {
                    
                    //window.alert("Successfully saved attribute: "+attrName"="+attrVal);
                    setEditSVGForm(svgFile);
                    setImgTable(svgFile);
            	},
            	error: function (error) {
                    window.alert("Error setting attribute!");
                    console.log(error);
            	}
            });
        }
    });


    $('#upload-form').submit(function(e){
        console.log("Form has fileName: "+$('#browse-upload').val());
        e.preventDefault();
        //Pass data to the Ajax call, so it gets passed to the server

        var formData = new FormData(this);

        $.ajax({
            type: 'post',
       		dataType: 'file',
        	url: '/upload',
        	data: formData,
            processData: false,
            contentType: false,
        	success: function (data) {
                window.alert("File was successfully uploaded");
                console.log(data);
        	},
        	fail: function (error) {
                window.alert("Error uploading this file!");
        	}
        });
    });

    $('#createSVG-form').submit(function(e){
        console.log("SVG name: "+$('#newSVGNameBox').val());
        e.preventDefault();

        svgNameBox = document.getElementById("newSVGNameBox");
        svgName = svgNameBox.value;

        svgTitleBox = document.getElementById("newSVGTitleBox");
        svgTitle = svgTitleBox.value;

        svgDescBox = document.getElementById("newSVGDescBox");
        svgDesc= svgDescBox.value;

        //Pass data to the Ajax call, so it gets passed to the server
        svgInfo = JSON.stringify('{"title":'+'"'+svgTitle+'","desc":'+'"'+svgDesc+'"}')

        $.ajax({
            type: 'get',
       		dataType: 'json',
        	url: '/createNewSVG',
        	data: {
        	   filePath: "./uploads/"+svgName,
               svgInfo: svgInfo,
        	},
        	success: function (data) {

                window.alert(data.status+": The image has been created!");
                refreshElement("svgList-table");
                //refreshElement("svg-dropdown");
                //refreshElement("add-dropdown");
                //refreshElement("edit-dropdown");

        	},
        	error: function (error) {
                window.alert("Error saving this image!");
        	}
        });
    });


    $('#addRect-form').submit(function(e){
        e.preventDefault();

        var compName;

        let svgLabel = document.getElementById("add-label-link");
        var svgFile = svgLabel.innerHTML;

        var new_x=parseInt($('#newRect_xEntryBox').val());

        var new_y=parseInt($('#newRect_yEntryBox').val());

        var new_width=parseInt($('#newRect_widthEntryBox').val());

        var new_height=parseInt($('#newRect_heightEntryBox').val());

        var new_units=$('#newRect_unitsEntryBox').val();

        //This must be added separately
        let new_fill = $('#rectFill').val();

        let rectJSON = JSON.stringify({x: new_x, y: new_y, width: new_width, height: new_height, units: new_units})

        console.log("Adding rect to "+svgFile);
        e.preventDefault();
        //Pass data to the Ajax call, so it gets passed to the server
        if(svgFile != '' && new_x!='' && new_y!=''&& new_width!=''&& new_height!=''){
            $.ajax({
                type: 'get',
           		dataType: 'json',
            	url: '/addRect',
            	data: {
                    filePath: "./uploads/"+svgFile,
            	    rectJSON: rectJSON,
            	},
            	success: function (data) {
                
                    compName = "Rectangle "+ data.rectNum;
                    console.log(compName +" "+new_fill+" "+svgFile);


                    if(new_fill != '' && compName != "Rectangle -1"){
                        addFill(compName, new_fill, svgFile);
                    }else{
                        window.alert("Error! An svg image must be selected, and X, Y, Width and Height boxes must have a value!");
                    }

                    refreshElement("svgList-table");

            	},
            	error: function (error) {
            
            	}
            });
        }else{
            window.alert("Error! An svg image must be selected, and X, Y, Width and Height boxes must have a value!");
        }

        
        function addFill(compName, new_fill, svgFile){
            console.log("compName: "+compName+" fill="+new_fill+" file: "+svgFile);
            $.ajax({
                type: 'get',
                dataType: 'json',
                url: '/editAttr',
                data: {
                    filePath: "./uploads/"+svgFile,
                    compName: compName,
                    attrName: "fill",
                    attrVal: new_fill,
                },
                success: function (data) {
                
                    refreshElement("svgList-table");

                },
                error: function (error) {
                    window.alert("Error adding fill attribute!");
                }
            });
        }
    });

    $('#addCirc-form').submit(function(e){
        e.preventDefault();

        var compName;

        let svgLabel = document.getElementById("add-label-link");
        var svgFile = svgLabel.innerHTML;

        var new_cx=parseInt($('#newCirc_xEntryBox').val());

        var new_cy=parseInt($('#newCirc_yEntryBox').val());

        var new_r=parseInt($('#newCirc_radiusEntryBox').val());

        var new_units=$('#newCirc_unitsEntryBox').val();

        //This must be added separately
        let new_fill = $('#circFill').val();

        let circJSON = JSON.stringify({cx: new_cx, cy: new_cy, r: new_r, units: new_units})

        console.log("Adding circ to "+svgFile);
        e.preventDefault();
        //Pass data to the Ajax call, so it gets passed to the server
        if(svgFile != '' && new_cx!='' && new_cy!=''&& new_r!=''){
            $.ajax({
                type: 'get',
                dataType: 'json',
                url: '/addCirc',
                data: {
                    filePath: "./uploads/"+svgFile,
                    circJSON: circJSON,
                },
                success: function (data) {
                
                    compName = "Circle "+ data.circNum;
                    console.log(compName +" "+new_fill+" "+svgFile);


                    if(new_fill != '' && compName != "Circle -1"){
                        addFill(compName, new_fill, svgFile);
                    }else{
                        window.alert("Error! An svg image must be selected, and X, Y, Width and Height boxes must have a value!");
                    }

                    refreshElement("svgList-table");

                },
                error: function (error) {
                    window.alert("Error adding circle!");
                    console.log(err);
                }
            });
        }else{
            window.alert("Error! An svg image must be selected, and X, Y, Width and Height boxes must have a value!");
        }

        
        function addFill(compName, new_fill, svgFile){
            console.log("compName: "+compName+" fill="+new_fill+" file: "+svgFile);
            $.ajax({
                type: 'get',
                dataType: 'json',
                url: '/editAttr',
                data: {
                    filePath: "./uploads/"+svgFile,
                    compName: compName,
                    attrName: "fill",
                    attrVal: new_fill,
                },
                success: function (data) {
                
                    refreshElement("svgList-table");

                },
                error: function (error) {
                    window.alert("Error adding fill attribute!");
                }
            });
        }
    });

    $('#rectScale-form').submit(function(e){
        e.preventDefault();

        let scaleFactor = parseFloat($('#scaleRectEntryBox').val());

        let scaleLabel = document.getElementById("scale-label");
        svgFile = scaleLabel.innerHTML;

        e.preventDefault();
        //Pass data to the Ajax call, so it gets passed to the server
        $.ajax({
            type: 'get',
       		dataType: 'json',
        	url: '/scaleRects',
        	data: {
                filePath: "./uploads/"+svgFile,
        	    scaleFactor: scaleFactor,
        	},
        	success: function (data) {
                
                window.alert("All rectangles for "+svgFile+ " have been scaled by x"+scaleFactor);
        
        	},
        	error: function (error) {
        
                window.alert("Error scaling rectangles!")
        	}
        });
    });

    $('#circScale-form').submit(function(e){
        e.preventDefault();

        let scaleFactor = parseFloat($('#scaleCircEntryBox').val());

        let scaleLabel = document.getElementById("scale-label");
        svgFile = scaleLabel.innerHTML;


        //Pass data to the Ajax call, so it gets passed to the server
        $.ajax({
            type: 'get',
       		dataType: 'json',
        	url: '/scaleCircs',
        	data: {
        	    filePath: "./uploads/"+svgFile,
                scaleFactor: scaleFactor,
        	},
        	success: function (data) {
                
                window.alert("All circles for "+svgFile+ " have been scaled by x"+scaleFactor);
        
        	},
        	error: function (error) {
                window.alert("Error scaling circles!")
        	}
        });
    });

    function refreshElement(elementID){

        var el = document.getElementById(elementID);
        var content = el.innerHTML;
        el.innerHTML= content;
  
        console.log("Refreshed"+elementID); 

    }
});