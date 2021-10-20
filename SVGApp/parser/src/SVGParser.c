/*  
 *  SVGParser.c
 *  Name: Matthew Ruetz
 *  Student #: 0961397
 * 
 */

#define LIBXML_SCHEMAS_ENABLED
#include <libxml/xmlschemastypes.h>
#include "SVGParser.h"
#include "ListBuilder.h"
#include <math.h>
#include <ctype.h>
#include <regex.h>

SVGimage* createSVGimage(char* fileName){
    
    xmlDoc *doc = NULL;
    xmlNode *root_element = NULL;
    SVGimage *newSVG = NULL;

    if (!fileName) //must include svg file name as argument
        return NULL;

    /*parse the file and get the DOM */
    doc = xmlReadFile(fileName, NULL, 0);
    
    if (doc == NULL) {
    	return NULL;

    }

    /*Get the root element node */
    root_element = xmlDocGetRootElement(doc);

    newSVG = build_SVGimage(root_element, NULL, newSVG);

    /*free the document */
    
    /*
     FOR SOME REASON I am getting memory leaks from xmlReadFile!!!
     */
    
    xmlFreeDoc(doc);
    xmlCleanupParser();

    return newSVG;

}

char* SVGimageToString(SVGimage* img){
    if(img == NULL) return NULL;
    
    size_t CHUNK = 512;
    char* svgStr = malloc(sizeof(char)*4056);
    strcpy(svgStr, "");
    
    
    strcat(svgStr, "\n====SVG Image====\nNamespace: ");
    strncat(svgStr, img->namespace, 256);
    printf("here2\n");
    strcat(svgStr, "\nTitle: ");
    strncat(svgStr, img->title, 256);
    strcat(svgStr, "\nDescription: ");
    strncat(svgStr, img->description, 256);
    
    if(strlen(svgStr) > 1500){
        svgStr = (char*)realloc(svgStr, sizeof(svgStr)+CHUNK);
    }
    //The strings for each List withing the svg struct are constructed first
    //This is done to reduce unexpected order of outputs due to recursion
    char* attrStr = toString(img->otherAttributes);
    char* rectStr = toString(img->rectangles);
    char* circleStr = toString(img->circles);
    char* pathStr = toString(img->paths);
    char* groupStr = toString(img->groups);

    strcat(svgStr, "\n\n----List of SVG Attributes----\n");
    strcat(svgStr, attrStr);
    free(attrStr);
    
    strcat(svgStr, "\n\n----List of Rectangles----\n");
    strcat(svgStr, rectStr);
    free(rectStr);
    
    strcat(svgStr, "\n\n----List of Circles----\n");
    strcat(svgStr, circleStr);
    free(circleStr);
    
    if(strlen(svgStr) > (2048 + CHUNK)){
        svgStr = (char*)realloc(svgStr, sizeof(svgStr)+CHUNK);
    }
    
    strcat(svgStr, "\n\n----List of Paths----\n");
    strcat(svgStr, pathStr);
    free(pathStr);
    
    
    strcat(svgStr, "\n\n----List of Groups----\n");
    strcat(svgStr, groupStr);
    free(groupStr);
    
    
    return svgStr;
    
}

void deleteSVGimage(SVGimage* img){
    if(img == NULL) return;
    
    strcpy(img->title, "");
    strcpy(img->description, "");
    strcpy(img->namespace, "");
    
    clearList(img->rectangles);
    clearList(img->circles);
    clearList(img->paths);
    clearList(img->groups);
    clearList(img->otherAttributes);
    
    freeList(img->rectangles);
    freeList(img->circles);
    freeList(img->paths);
    freeList(img->groups);
    freeList(img->otherAttributes);
           
    free(img);    
}


// Function that returns a list of all rectangles in the image.  
List* getRects(SVGimage* img){
    if(img == NULL) return NULL;
    
    List* rectList = initializeList(rectangleToString, deleteRectangle, compareRectangles);
    Rectangle* elem = NULL;
    ListIterator iter = createIterator(img->rectangles);

    //Loop iterates through SVG rectangle list and adds pointers to rectList
    while((elem = (Rectangle*)nextElement(&iter)) != NULL){
        
        insertBack(rectList, elem);
        
    }
    
    List* rectListFromGroups = initializeList(rectangleToString, deleteRectangle, compareRectangles);
    getRectListFromGroups(img->groups, NULL, rectListFromGroups);

    if(rectListFromGroups->length > 0){
        //creating a human centipede... (stitching lists)
        if(rectList->length == 0){ //If SVG has no rects but groups do...
            free(rectList);
            rectList = rectListFromGroups;
        }
        else{ //append rects in groups list to SVG rects list
            rectListFromGroups->head->previous = rectList->tail;
            rectList->tail->next = rectListFromGroups->head;
            rectList->tail = rectListFromGroups->tail;
            rectList->length = (rectList->length + rectListFromGroups->length);
            
            free(rectListFromGroups);
        }
    }
    else{
        freeNodesInList(rectListFromGroups);
        free(rectListFromGroups);
    }
    return rectList;
    
}


// Function that returns a list of all circles in the image.  
List* getCircles(SVGimage* img){
    if(img == NULL) return NULL;
    
    List* circleList = initializeList(circleToString, deleteCircle, compareCircles);
    Circle* elem = NULL;
    ListIterator iter = createIterator(img->circles);

    //Loop iterates through SVG circle list and adds pointers to List
    while((elem = (Circle*)nextElement(&iter)) != NULL){
        
        insertBack(circleList, elem);
    }

    List* circleListFromGroups = initializeList(circleToString, deleteCircle, compareCircles);
    
    getCircleListFromGroups(img->groups, NULL, circleListFromGroups);
    
    if(circleListFromGroups->length > 0){
        
        if(circleList->length == 0){ //If SVG has no circles but groups do...
            free(circleList);
            circleList = circleListFromGroups;
            
        }
        else{ //append circles in groups list to SVG circles list
            circleListFromGroups->head->previous = circleList->tail;
            circleList->tail->next = circleListFromGroups->head;
            circleList->tail = circleListFromGroups->tail;
            circleList->length = (circleList->length + circleListFromGroups->length);
            
            free(circleListFromGroups);
        }    
    }
    else{
        freeNodesInList(circleListFromGroups);
        free(circleListFromGroups);
    }
    
    
    return circleList;

}


// Function that returns a list of all groups in the image.  
List* getGroups(SVGimage* img){
    if(img == NULL) return NULL;
    
    List* groupList = initializeList(groupToString, deleteGroup, compareGroups);
    Group* elem = NULL;
    ListIterator iter = createIterator(img->groups);

    //Loop iterates through SVG group list and adds pointers to groupList
    while((elem = (Group*)nextElement(&iter)) != NULL){
        
        insertBack(groupList, elem);
    }
    
    List* groupListFromGroups = initializeList(groupToString, deleteGroup, compareGroups);
    
    getGroupListFromGroups(img->groups, NULL, groupListFromGroups);

    if(groupListFromGroups->length > 0){
        //creating a human centipede... (stitching lists)
        if(groupList->length == 0){ //If SVG has no groups but rects do...
            free(groupList);
            groupList = groupListFromGroups;
        }
        else{ //append groups in groups list to SVG groups list
            groupList->head->previous = groupList->tail;
            groupList->tail->next = groupListFromGroups->head;
            groupList->tail = groupListFromGroups->tail;
            groupList->length = (groupList->length + groupListFromGroups->length);
            
            free(groupListFromGroups);
        }
    }
    else{
        freeNodesInList(groupListFromGroups);
        free(groupListFromGroups);
    }
    
    return groupList;

}


// Function that returns a list of all paths in the image.  
List* getPaths(SVGimage* img){
    if(img == NULL) return NULL;
    
    List* pathList = initializeList(pathToString, deletePath, comparePaths);
    Path* elem = NULL;
    ListIterator iter = createIterator(img->paths);

    //Loop iterates through SVG path list and adds pointers to pathList
    while((elem = (Path*)nextElement(&iter)) != NULL){
        
        insertBack(pathList, elem);
    }

    List* pathListFromGroups = initializeList(pathToString, deletePath, comparePaths);
    
    getPathListFromGroups(img->groups, NULL, pathListFromGroups);
    
    if(pathListFromGroups->length > 0){
        //creating a human centipede... (stitching lists)
        if(pathList->length == 0){ //If SVG has no paths but groups do...
            free(pathList);
            pathList = pathListFromGroups;
        }
        else{ //append paths in groups list to SVG paths list
            pathListFromGroups->head->previous = pathList->tail;
            pathList->tail->next = pathListFromGroups->head;
            pathList->tail = pathListFromGroups->tail;
            pathList->length = (pathList->length + pathListFromGroups->length);
            
            free(pathListFromGroups);
        }

    }
    else{
        freeNodesInList(pathListFromGroups);
        free(pathListFromGroups);
    }

    return pathList;

}





// Function that returns the number of all rectangles with the specified area
int numRectsWithArea(SVGimage* img, float area){
    if(img == NULL || area < 0) return 0;
    
    area = ceil(area); //Round up float for comparison
    List* fullRectList = getRects(img);
    
    if(fullRectList->length == 0){
        free(fullRectList);
        return 0;
    }

    float rectArea = 0;
    int numRects = 0;
    
    ListIterator iter = createIterator(fullRectList);
    Rectangle* elem = NULL;
        
    while((elem = (Rectangle*)nextElement(&iter)) != NULL){

        rectArea = ceil(elem->height*elem->width);
        
        if(rectArea == area){
            ++numRects;
        }

    }
    Node* curNodeToFree = fullRectList->head;
    Node* nextNodeToFree = NULL;
    
    while(curNodeToFree != NULL){
        nextNodeToFree = curNodeToFree->next;
        
        free(curNodeToFree);
        
        curNodeToFree = nextNodeToFree;
    }
    freeNodesInList(fullRectList);
    free(fullRectList);
    
    return numRects;
}
// Function that returns the number of all circles with the specified area
int numCirclesWithArea(SVGimage* img, float area){
    if(img == NULL || area < 0) return 0;

    area = ceil(area); //Round up float for comparison
    List* fullCircleList = getCircles(img);
    
    if(fullCircleList->length == 0){
        free(fullCircleList);
        return 0;
    }
    
    float circleArea = 0;
    int numCircles = 0;
    
    ListIterator iter = createIterator(fullCircleList);
    Circle* elem = NULL;
        
    while((elem = (Circle*)nextElement(&iter)) != NULL){

        circleArea = ceil((3.14159)*elem->r*elem->r);
        
        if(circleArea == area){
            ++numCircles;
        }

    }
    freeNodesInList(fullCircleList);
    free(fullCircleList);
    
    return numCircles;

}
// Function that returns the number of all paths with the specified data - i.e. Path.data field
int numPathsWithdata(SVGimage* img, char* data){
    if(img == NULL || data == NULL || strlen(data) < 2) return 0;
    
    List* fullPathList = getPaths(img);
    
    if(fullPathList->length == 0){
        free(fullPathList);
        return 0;
    }
    
    char* pathData = NULL;
    int numPaths = 0;
    
    ListIterator iter = createIterator(fullPathList);
    Path* elem = NULL;
        
    while((elem = (Path*)nextElement(&iter)) != NULL){

        pathData = elem->data;
        
        if(strcmp(pathData, data) == 0){
            ++numPaths;
        }

    }
    freeNodesInList(fullPathList);
    free(fullPathList);
    
    return numPaths;

}
    
// Function that returns the number of all groups with the specified length - see A1 Module 2 for details
int numGroupsWithLen(SVGimage* img, int len){
    if(img == NULL || len < 0) return 0;

    List* fullGroupList = getGroups(img);
    
    if(fullGroupList->length == 0){
        free(fullGroupList);
        return 0;
    }
    
    int numGroups = 0;
    int groupLen = 0;
    
    ListIterator iter = createIterator(fullGroupList);
    Group* elem = NULL;
        
    while((elem = (Group*)nextElement(&iter)) != NULL){

        groupLen = elem->rectangles->length +
                elem->circles->length +
                elem->paths->length + 
                elem->groups->length;
        
        if(len == groupLen){
            ++numGroups;
        }
        
    }
    freeNodesInList(fullGroupList);
    free(fullGroupList);
    
    return numGroups;
}

int numAttr(SVGimage* img){
    if(img == NULL) return 0;
    
    //First adding number of SVG attributes
    int totalNumAttr = img->otherAttributes->length;
    
    //Adding attributes from all rectangles
    List* rectList = getRects(img);
    {
        ListIterator iter = createIterator(rectList);
        Rectangle* elem = NULL;
        
        while((elem = (Rectangle*)nextElement(&iter)) != NULL){
            
            totalNumAttr += elem->otherAttributes->length;
        }
        freeNodesInList(rectList);
        free(rectList);
    }
    
    //Adding attributes from all circles
    List* circleList = getCircles(img);
    {
        ListIterator iter = createIterator(circleList);
        Circle* elem = NULL;
        
        while((elem = (Circle*)nextElement(&iter)) != NULL){
            
            totalNumAttr += elem->otherAttributes->length;
        }
        freeNodesInList(circleList);
        free(circleList);
    }
    
    //Adding attributes from all paths
    List* pathList = getPaths(img);
    {
        ListIterator iter = createIterator(pathList);
        Path* elem = NULL;
        
        while((elem = (Path*)nextElement(&iter)) != NULL){
            
            totalNumAttr += elem->otherAttributes->length;
        }
        freeNodesInList(pathList);
        free(pathList);
    }
    
    //Adding attributes from all groups
    List* groupList = getGroups(img);
    {
        ListIterator iter = createIterator(groupList);
        Group* elem = NULL;
        
        while((elem = (Group*)nextElement(&iter)) != NULL){
            
            totalNumAttr += elem->otherAttributes->length;
        }
        freeNodesInList(groupList);
        free(groupList);
    }
    
    return totalNumAttr;
    
}


//-----------LIST HELPER FUNCTIONS----------------------

//For Attributes ---------------------------------------------------------------
void deleteAttribute(void* data){
    if(data == NULL) return;
    
    Attribute * attrToDelete = (Attribute *)data;
    
    //set string contents to '0' (empty)
    strcpy(attrToDelete->name, "");
    strcpy(attrToDelete->value, "");
    
    //free memory
    free(attrToDelete->name);
    free(attrToDelete->value);
    free(attrToDelete);
    //free(data);        //NOT SURE IF NEEDED
}

char* attributeToString(void* data){
    if(data == NULL) return NULL;
    
    Attribute * attrToStr = (Attribute *)data;
    
    char* attrStr = NULL;
    
    attrStr = malloc(sizeof(char)*(strlen(attrToStr->name)+ strlen(attrToStr->value) + 200));
    strcpy(attrStr, "");
    
    strcat(attrStr, "Attribute name: ");
    strcat(attrStr, attrToStr->name);
    strcat(attrStr, "  |  value: ");
    strcat(attrStr, attrToStr->value);
    
   // strcat(attrStr, attrToJSON(attrToStr));
    
    return attrStr;
}

int compareAttributes(const void *first, const void *second){
    if(first == NULL || second == NULL) return 0;
    
    Attribute * attr1 = (Attribute *) first;
    Attribute * attr2 = (Attribute *) second;
    
    return(attr2->name - attr1->name);
    
    return 0;
}

//For groups -------------------------------------------------------------------
void deleteGroup(void* data){
    if(data == NULL) return;

    Group * groupToDelete = (Group*) data;
    
   
    clearList(groupToDelete->rectangles);
    clearList(groupToDelete->circles);
    clearList(groupToDelete->paths);
    clearList(groupToDelete->otherAttributes);
    
    freeList(groupToDelete->rectangles);
    freeList(groupToDelete->circles);
    freeList(groupToDelete->paths);
    freeList(groupToDelete->otherAttributes);
 
    clearList(groupToDelete->groups);
    freeList(groupToDelete->groups);

    free(groupToDelete);
    
}

char* groupToString( void* data){ //WORK ON THIS AND FIGURE OUT SYNTAX
    if(data == NULL) return NULL;
    Group* groupToStr = (Group *) data;
    char* groupStr = malloc(sizeof(char) * 2048);
    strcpy(groupStr, "--Group--\n");
    
    char* attrStr = toString(groupToStr->otherAttributes);
    char* rectStr = toString(groupToStr->rectangles);
    char* circleStr = toString(groupToStr->circles);
    char* pathStr = toString(groupToStr->paths);
    
    char nestedGroupCt[5];
    if(snprintf(nestedGroupCt, 5, "%d", groupToStr->groups->length) != -1){
        strcat(groupStr, "Nested groups: ");
        strcat(groupStr, nestedGroupCt);
    }
    
    strcat(attrStr, "\n");
    strcat(groupStr, attrStr);
    free(attrStr);
    strcat(groupStr, pathStr);
    free(pathStr);
    strcat(groupStr, circleStr);
    free(circleStr);
    strcat(groupStr, rectStr);
    free(rectStr);
    
    strcat(groupStr, "\n--End of Group--\n\n");
    
    return groupStr;
    
}

int compareGroups(const void *first, const void *second){
    return 0;
}

//For rectangles ---------------------------------------------------------------
void deleteRectangle(void* data){
    if(data == NULL) return;

    Rectangle * rectToDelete = (Rectangle *) data;
    
    rectToDelete->height = 0;
    rectToDelete->width = 0;
    rectToDelete->x = 0;
    rectToDelete->y = 0;
    
    strcpy(rectToDelete->units, "");
    
    clearList(rectToDelete->otherAttributes);
    freeList(rectToDelete->otherAttributes);
    
    free(rectToDelete);
    
}

char* rectangleToString(void* data){
    if(data == NULL) return NULL;
    
    Rectangle * rectToStr = (Rectangle *) data;
    
    char* rectStr = (char*)malloc(sizeof(char)*1000);
    strcpy(rectStr, "");
    
    char temp[100];
    strcpy(temp, "");
    
    //It tastes awful, but it works
    
    strcat(rectStr, "type: Rectangle\nx: ");
    sprintf(temp, "%.3f", rectToStr->x);
    strcat(rectStr, temp);
    strcat(rectStr, "  |  y: ");
    sprintf(temp, "%.3f", rectToStr->y);
    strcat(rectStr, temp);
    strcat(rectStr, "  |  width: ");
    sprintf(temp, "%.3f", rectToStr->width);
    strcat(rectStr, temp);
    strcat(rectStr, "  |  height: ");
    sprintf(temp, "%.3f", rectToStr->height);
    strcat(rectStr, temp);
    strcat(rectStr, "  |  units: ");
    strcat(rectStr, rectToStr->units);
    strcat(rectStr, "\nOther Attributes:");
    
    char* oA_str = toString(rectToStr->otherAttributes);
    
    strcat(rectStr, oA_str);
    free(oA_str);
    
    return rectStr;
    
}

int compareRectangles(const void *first, const void *second){
    if(first == NULL || second == NULL) return 0;
    
    Rectangle * rect1 = (Rectangle *) first;
    Rectangle * rect2 = (Rectangle *) second;
    
    //Sorts based on x parameter
    return(rect2->x - rect1->x);
    
    return 0;
}

//For circles ------------------------------------------------------------------
void deleteCircle(void* data){
    if(data == NULL) return;
    
    Circle * circleToDelete = (Circle *) data;
    
    circleToDelete->cx = 0;
    circleToDelete->cy = 0;
    circleToDelete->r = 0;
    
    strcpy(circleToDelete->units, "");
    
    clearList(circleToDelete->otherAttributes);
    freeList(circleToDelete->otherAttributes);
    
    free(circleToDelete);
    
}

char* circleToString(void* data){
    if(data == NULL) return NULL;
    
    Circle * circleToStr = (Circle *) data;
    
    char* circleStr = (char*)malloc(sizeof(char)*1000);
    strcpy(circleStr, "");
    
    char temp[100];
    
    strcat(circleStr, "type: Circle\n cx: ");
    sprintf(temp, "%.3f", circleToStr->cx);
    strcat(circleStr, temp);
    strcat(circleStr, "  |  cy: ");
    sprintf(temp, "%.3f", circleToStr->cy);
    strcat(circleStr, temp);
    strcat(circleStr, "  |  r: ");
    sprintf(temp, "%.3f", circleToStr->r);
    strcat(circleStr, temp);
    strcat(circleStr, "  |  units: ");
    strcat(circleStr, circleToStr->units);
    strcat(circleStr, "\nOther Attributes:");
    
    char* oA_str = toString(circleToStr->otherAttributes);
    
    strcat(circleStr, oA_str);
    
    free(oA_str);
    
    
    return circleStr;
}

int compareCircles(const void *first, const void *second){
    if(first == NULL || second == NULL) return 0;
    
    Circle * circle1 = (Circle *) first;
    Circle * circle2 = (Circle *) second;
    
    return(circle2->r - circle1->r);

}

//For Paths --------------------------------------------------------------------
void deletePath(void* data){
    if(data == NULL) return;
    
    Path * pathToDelete = (Path *) data;
    
    strcpy(pathToDelete->data, "");
    free(pathToDelete->data);
    
    clearList(pathToDelete->otherAttributes);
    freeList(pathToDelete->otherAttributes);
    
    free(pathToDelete);
}

char* pathToString(void* data){
    if(data == NULL) return NULL;
    
    Path * pathToStr = ((Path *) data);
    
    char* pathStr = (char*)malloc(sizeof(char)*(strlen(pathToStr->data)+512));
    strcpy(pathStr, "");
    
    strcat(pathStr, "type: Path\n data: ");
    strncat(pathStr, pathToStr->data, strlen(pathToStr->data));
    
    strcat(pathStr, "\nOther Attributes:");
    
    char* oA_str = toString(pathToStr->otherAttributes);
    strcat(pathStr, oA_str);
    
    free(oA_str);

    return pathStr;
    
}

int comparePaths(const void *first, const void *second){
    if(first == NULL || second == NULL) return 0;
    
    Path * path1 = (Path *) first;
    Path * path2 = (Path *) second;
    
    return(path2->data - path1->data);
    
    return 0;
}

//---------------------------Additional Helper Functions------------------------

//This method is derived from the xml example file "tree1.c", created by Dodji
//Seketeli, with changes made to suit the needs of this assignment.
SVGimage* build_SVGimage(xmlNode * a_node, Group * parentGroup, SVGimage * newSVG){
    if(a_node == NULL) return NULL;
    
    xmlNode *cur_node = NULL;
    Group* newGroup = NULL;
    int i = 0;
    
    for (cur_node = a_node; cur_node; cur_node = cur_node->next) {
            
        if (cur_node->type == XML_ELEMENT_NODE){ //circle, rect, path, and g
        
            if(strcmp((char*)cur_node->name, "svg") == 0){

                newSVG = parseSVG(cur_node);
                getTitleAndDesc(cur_node, newSVG);
                
            }
            else if(strcmp((char*)cur_node->name, "rect") == 0){
                Rectangle* newRect = parseRect(cur_node);
                
                if(parentGroup != NULL){
                    insertBack(parentGroup->rectangles, newRect);
                }else{
                    insertBack(newSVG->rectangles, newRect);
                }
                
            }
            else if(strcmp((char*)cur_node->name, "circle") == 0){
                Circle* newCircle = parseCircle(cur_node); 
                
                if(parentGroup != NULL){
                    insertBack(parentGroup->circles, newCircle);
                }else{
                    insertBack(newSVG->circles, newCircle);
                }
                
            }
            else if(strcmp((char*)cur_node->name, "path") == 0){
                Path* newPath = parsePath(cur_node);
                
                if(parentGroup != NULL){
                    insertBack(parentGroup->paths, newPath);
                }else{
                    insertBack(newSVG->paths, newPath);
                }
                
            }
            else if(strcmp((char*)cur_node->name, "g") == 0){
                
                newGroup = initializeGroup(cur_node);
                
                if(parentGroup != NULL){
                    insertBack(parentGroup->groups, newGroup);
                }else{
                    ++i;
                    insertBack(newSVG->groups, newGroup);
                }
                
            }
        }
        build_SVGimage(cur_node->children, newGroup, newSVG);
    }
    return newSVG;
}



void getTitleAndDesc(xmlNode* svgNode, SVGimage* newSVG){
    if(svgNode == NULL || newSVG == NULL) return;
    
    xmlNode* cur_node = NULL;
    
    for (cur_node = svgNode; cur_node; cur_node = cur_node->next) {
        if (cur_node->type == XML_ELEMENT_NODE){
            
            if(strcmp((char*)cur_node->name, "title") == 0){
                strncpy(newSVG->title, (char*)cur_node->children->content, 256);
            }
            else if(strcmp((char*)cur_node->name, "desc") == 0){
                strncpy(newSVG->description, (char*)cur_node->children->content, 256);
            }
        }
        
        getTitleAndDesc(cur_node->children, newSVG);
    }
}



SVGimage * initializeSVG(){
    
    SVGimage * newSVG = malloc(sizeof(SVGimage));
    
    strcpy(newSVG->namespace, "");
    strcpy(newSVG->description, "");
    strcpy(newSVG->title, "");
    
    newSVG->rectangles = initializeList(rectangleToString, deleteRectangle, compareRectangles);
    newSVG->circles = initializeList(circleToString, deleteCircle, compareCircles);
    newSVG->paths = initializeList(pathToString, deletePath, comparePaths);
    newSVG->groups = initializeList(groupToString, deleteGroup, compareGroups);
    newSVG->otherAttributes = initializeList(attributeToString, deleteAttribute,compareAttributes);
    
    return newSVG;
}


SVGimage * parseSVG(xmlNode * newNode){
    if(newNode == NULL) return NULL;
    
    SVGimage* newSVG = initializeSVG(); //MAKE SURE THIS GETS FREED
    
    strncpy(newSVG->namespace, (char*)newNode->ns->href, 256);
    
    xmlAttr* attrList = newNode->properties;
    
    while(1){
        Attribute * curAttr = initializeAttribute(attrList);
        
        insertBack(newSVG->otherAttributes, curAttr); 
        
        if(attrList->next != NULL){
            attrList = attrList->next;
        }
        else break;
    }

    return newSVG;
}


Rectangle* parseRect(xmlNode * newNode){
    if(newNode == NULL) return NULL;
    
    Rectangle* newRect = malloc(sizeof(Rectangle));
    strcpy(newRect->units, "");
    
    
    
    newRect->otherAttributes = initializeList(attributeToString, deleteAttribute,compareAttributes);

    xmlAttr* attrList = newNode->properties;
    
    while(1){
        
        Attribute * curAttr = initializeAttribute(attrList);
        
        if(strlen(newRect->units) == 0 && isMeasurement(curAttr->name)){
            
            char* units = getUnits(curAttr->value);
            
            if(strcmp(units, "none") != 0){
                strcpy(newRect->units, units);
            }
            
            free(units);
        }
        
        
        if(strcmp(curAttr->name, "x") == 0 && curAttr->value != NULL){ //Not sure if 2nd part is needed
            newRect->x = strtof(curAttr->value, NULL);
            deleteAttribute(curAttr);
        }
        else if(strcmp(curAttr->name, "y") == 0 && curAttr->value != NULL){
            newRect->y = strtof(curAttr->value, NULL);
            deleteAttribute(curAttr);
        }
        else if(strcmp(curAttr->name, "height") == 0 && curAttr->value != NULL){
            
            if(strtof(curAttr->value, NULL) >= 0){
                newRect->height = strtof(curAttr->value, NULL);
            } else newRect->height = 0;
            deleteAttribute(curAttr);
        }
        else if(strcmp(curAttr->name, "width") == 0 && curAttr->value != NULL){
            
            if(strtof(curAttr->value, NULL) >= 0){
                newRect->width = strtof(curAttr->value, NULL);
            } else newRect->width = 0;
            deleteAttribute(curAttr);
        }
        else{
            insertBack(newRect->otherAttributes, curAttr); 
        }
        
        
        if(attrList->next != NULL){
            attrList = attrList->next;
        }
        else{
            break;
        }
    }
    
    return newRect;
    
}



Circle* parseCircle(xmlNode * newNode){
    if(newNode == NULL) return NULL;
    
    Circle* newCircle = malloc(sizeof(Circle)); //MAKE SURE THIS GETS FREED
    strcpy(newCircle->units, "");
    
    newCircle->otherAttributes = initializeList(attributeToString, deleteAttribute, compareAttributes);

    
    xmlAttr* attrList = newNode->properties;
    
    while(1){
        
        Attribute * curAttr = initializeAttribute(attrList);
        
        //First check if this attribute has units, and set struct units value
        if(strlen(newCircle->units) < 1 && isMeasurement(curAttr->name)){
            
            char* units = getUnits(curAttr->value);
            if(strcmp(units, "none") != 0){
                strcpy(newCircle->units, units);
            }
            free(units);
        }
        
        if(strcmp(curAttr->name, "cx") == 0 && curAttr->value != NULL){ //Not sure if 2nd part is needed
            newCircle->cx = strtof(curAttr->value, NULL);
            deleteAttribute(curAttr);
        }
        else if(strcmp(curAttr->name, "cy") == 0 && curAttr->value != NULL){
            newCircle->cy = strtof(curAttr->value, NULL);
            deleteAttribute(curAttr);
        }
        else if(strcmp(curAttr->name, "r") == 0 && curAttr->value != NULL){
            //assuring that radius >= 0
             if(strtof(curAttr->value, NULL) >= 0){
                newCircle->r = strtof(curAttr->value, NULL);
            } else newCircle->r = 0;
             
            deleteAttribute(curAttr);
        }
        else{
            insertBack(newCircle->otherAttributes, curAttr); 
        }
        
        
        if(attrList->next != NULL){
            
            attrList = attrList->next;
        }
        else break;
    }

    return newCircle;
    
}


Path* parsePath(xmlNode * newNode){
    if(newNode == NULL) return NULL;
    
    Path* newPath = malloc(sizeof(Path));
    
    newPath->otherAttributes = initializeList(attributeToString,deleteAttribute,compareAttributes);
    
    xmlAttr* attrList = newNode->properties;
    
    while(1){
        
        Attribute * curAttr = initializeAttribute(attrList);
        
        
        if(strcmp(curAttr->name, "d") == 0 && curAttr->value != NULL){ //Not sure if 2nd part is needed
            
            newPath->data = curAttr->value;
            free(curAttr->name);
            free(curAttr);
            
            strcat(newPath->data, "\0");
            
        }
        else{
            insertBack(newPath->otherAttributes, curAttr);
        }
        
        if(attrList->next != NULL){
            
            attrList = attrList->next;
        }
        else break;
    }

    return newPath;
}

Group* initializeGroup(xmlNode* cur_node){
    if(cur_node == NULL) return NULL;
    
    Group* newGroup = malloc(sizeof(Group));
    
    newGroup->rectangles = initializeList(rectangleToString, deleteRectangle, compareRectangles);
    newGroup->circles = initializeList(circleToString, deleteCircle, compareCircles);
    newGroup->paths = initializeList(pathToString, deletePath, comparePaths);
    newGroup->groups = initializeList(groupToString, deleteGroup, compareGroups);
    newGroup->otherAttributes = initializeList(attributeToString, deleteAttribute, compareAttributes);
    
    xmlAttr* attrList = cur_node->properties;
    
    while(attrList != NULL){
        
        Attribute * curAttr = initializeAttribute(attrList);
        
        insertBack(newGroup->otherAttributes, curAttr);
        
        attrList = attrList->next;
        
    }
    
    
    return newGroup;
    
}

char* getUnits(const char* val){
    if(val == NULL) return NULL;
    
    char temp[50];
    strcpy(temp, "none");
    
    char* units = malloc(sizeof(char)*50);
    strcpy(units, "none");
    
    int i;
    int unitsIndex = 0;
    
    for(i=0; i < strlen(val) && unitsIndex < 50; ++i){
        
        if(isalpha(val[i])){
            if(strcmp(temp, "none")==0)memset(temp, 0, 50);
            
            temp[unitsIndex] = val[i];
            ++unitsIndex;
        }
    }
    if(strcmp(temp, "none") != 0){
        strncpy(units, temp, 50);
    }
    
    return units;
}


Attribute* initializeAttribute(xmlAttr * newAttr){
    if(newAttr == NULL) return NULL;
    
    Attribute * curAttr = malloc(sizeof(Attribute));
    
    curAttr->name = malloc(sizeof(char)*strlen((char*)newAttr->name)+30);
    strcpy(curAttr->name, "");
    
    curAttr->value = malloc(sizeof(char)*strlen((char*)newAttr->children->content)+30);
    strcpy(curAttr->value, "");
    
    strcpy(curAttr->name, (char*)newAttr->name);
    strcpy(curAttr->value, (char*)newAttr->children->content);
    
    return curAttr;
    
}

int isMeasurement(char* val){
    if(val == NULL) return -1;
    
    if(strcmp(val, "cx") == 0 ||
            strcmp(val, "cy") == 0 ||
            strcmp(val, "r") == 0 ||
            strcmp(val, "pathLength") == 0 ||
            strcmp(val, "x") == 0 ||
            strcmp(val, "y") == 0 ||
            strcmp(val, "width") == 0 ||
            strcmp(val, "height") == 0 ||
            strcmp(val, "rx") == 0 ||
            strcmp(val, "ry") == 0 ||
            strcmp(val, "d") == 0)
    {
        return 1;
    }
    return 0;
}

/* The following functions are used to recursively build lists of elements
 * found in groups. These lists will be appended to the list of such elements
 * within the SVG image that are not in groups.
 */

List * getRectListFromGroups(List* groupList, Node* curGroupNode, List * rectangleList){
    
    //test whether given list of groups is empty - return NULL if empty
    if(groupList->length == 0){
        return NULL;
    }
    
    //test whether given Group Node is unspecified - set to head of list if so
    if(curGroupNode == NULL){
        curGroupNode = groupList->head;
    }
    
    Group* curGroup = (Group*)curGroupNode->data;
    
    //If group has nested groups, recur inward (depth first)
    if(curGroup->groups->head != NULL){
        getRectListFromGroups(curGroup->groups, NULL, rectangleList);
    }
    
    //Use iterator to add this group's rectangles to list
    if(curGroup->rectangles->length != 0){
        
        List* groupRectangleList = curGroup->rectangles;
        ListIterator iterInGroup = createIterator(groupRectangleList);
        Rectangle* elem = NULL;
        
        //iterates through rectangle list of current group
        while((elem = (Rectangle*)nextElement(&iterInGroup)) != NULL){
            insertBack(rectangleList, elem);
        }
        
    }
    
    //recur to next group within this group list if there are more (breadth)
    if(curGroupNode->next != NULL){
        getRectListFromGroups(groupList, curGroupNode->next, rectangleList);
    }
    
    return rectangleList;
}


List * getCircleListFromGroups(List* groupList, Node* curGroupNode, List * circleList){
    
    //test whether given list of groups is empty - return NULL if empty
    if(groupList->length == 0){
        return NULL;
    }
    
    //test whether given Group Node is unspecified - set to head of list if so
    if(curGroupNode == NULL){
        curGroupNode = groupList->head;
    }
    
    Group* curGroup = (Group*)curGroupNode->data;
    
    //If group has nested groups, recur inward (depth first)
    if(curGroup->groups->head != NULL){
        getCircleListFromGroups(curGroup->groups, NULL, circleList);
    }
    
    //Use iterator to add this group's circles to list
    if(curGroup->circles->length != 0){
        
        List* groupCircleList = curGroup->circles;
        ListIterator iterInGroup = createIterator(groupCircleList);
        Circle* elem = NULL;
        
        //iterates through circle list of current group
        while((elem = (Circle*)nextElement(&iterInGroup)) != NULL){
            insertBack(circleList, elem);
        }
        
    }
    
    //recur to next group within this group list if there are more (breadth)
    if(curGroupNode->next != NULL){
        getCircleListFromGroups(groupList, curGroupNode->next, circleList);
    }
    
    return circleList;
}


List * getGroupListFromGroups(List* groupList, Node* curGroupNode, List* nestedGroupList){
    
    
    if(groupList->length == 0){
        return NULL;
    }
    
    if(curGroupNode == NULL){
        curGroupNode = groupList->head;
    }

    Group* curGroup = (Group*)curGroupNode->data;
    
    //If group has nested groups, recur inward (depth first)
    if(curGroup->groups->head != NULL){
        getGroupListFromGroups(curGroup->groups, NULL, nestedGroupList);
    }

    //Use iterator to add this group's groups to list
    if(curGroup->groups->length != 0){
        
        List* groupGroupList = curGroup->groups;
        ListIterator iterInGroup = createIterator(groupGroupList);
        Group* elem = NULL;
        
        //iterates through group list of current group
        while((elem = (Group*)nextElement(&iterInGroup)) != NULL){
            insertBack(nestedGroupList, elem);
        }
    }
    
    if(curGroupNode->next != NULL){
        getGroupListFromGroups(groupList, curGroupNode->next, nestedGroupList);
    }

    return nestedGroupList;
}


List * getPathListFromGroups(List* groupList, Node* curGroupNode, List * pathList){
    
    //test whether given list of groups is empty - return NULL if empty
    if(groupList->length == 0){
        return NULL;
    }
    
    //test whether given Group Node is unspecified - set to head of list if so
    if(curGroupNode == NULL){
        curGroupNode = groupList->head;
    }
    
    Group* curGroup = (Group*)curGroupNode->data;
    
    //If group has nested groups, recur inward (depth first)
    if(curGroup->groups->head != NULL){
        getPathListFromGroups(curGroup->groups, NULL, pathList);
    }
    
    //Use iterator to add this group's paths to list
    if(curGroup->paths->length != 0){
        
        List* groupPathList = curGroup->paths;
        ListIterator iterInGroup = createIterator(groupPathList);
        Path* elem = NULL;
        
        //iterates through path list of current group
        while((elem = (Path*)nextElement(&iterInGroup)) != NULL){
            insertBack(pathList, elem);
        }
        
    }
    
    //recur to next group within this group list if there are more (breadth)
    if(curGroupNode->next != NULL){
        getPathListFromGroups(groupList, curGroupNode->next, pathList);
    }
    
    return pathList;
}


void freeNodesInList(List* toFreeFrom){
    
    Node* curNode = toFreeFrom->head;
    
    for(int i = 0; i < toFreeFrom->length - 1; ++i){

        curNode = curNode->next;
        free(curNode->previous);
    }
    free(curNode);
}

//-------------------- ASSIGNMENT 2 FUNCTIONS ----------------------

bool validateSVGimage(SVGimage* image, char* schemaFile){
    if(image == NULL || schemaFile == NULL || strlen(schemaFile) == 0) return 0;
    //1. Must satisfy specs in SVGParser.h (eg. no negative radius, null pointers, etc.)
        //The following blocks verify validity according to SVGParser.h
    
    
    List* rectList = getRects(image);
    {
        if(rectList == NULL) return 0;
        ListIterator iter = createIterator(rectList);
        Rectangle* elem = NULL;
        
        while((elem = (Rectangle*)nextElement(&iter)) != NULL){

            if(elem->height < 0 || elem->width < 0 || elem->otherAttributes == NULL || elem->units == NULL ||
                    validateOtherAttributes(elem->otherAttributes) == 0){
                
                return 0;
            }

        }
        freeNodesInList(rectList);
        free(rectList);
    }
    List* circleList = getCircles(image);
    {
        if(circleList == NULL) return 0;
        ListIterator iter = createIterator(circleList);
        Circle* elem = NULL;
        
        while((elem = (Circle*)nextElement(&iter)) != NULL){

            if(elem->r < 0 || elem->otherAttributes == NULL || elem->units == NULL ||
                    validateOtherAttributes(elem->otherAttributes) == 0){
                
                return 0;
            }
            
        }
        freeNodesInList(circleList);
        free(circleList);
    }
    List* pathList = getPaths(image);
    {
        if(pathList == NULL) return 0;
        ListIterator iter = createIterator(pathList);
        Path* elem = NULL;
        
        while((elem = (Path*)nextElement(&iter)) != NULL){

            if(elem->data == NULL || elem->otherAttributes == NULL ||
                    validateOtherAttributes(elem->otherAttributes) == 0){
                return 0;
            }
        }
        freeNodesInList(pathList);
        free(pathList);
    }
    List* groupList = getGroups(image);
    {
        if(groupList == NULL) return 0;
        ListIterator iter = createIterator(groupList);
        Group* elem = NULL;
        
        while((elem = (Group*)nextElement(&iter)) != NULL){
            if(elem->rectangles == NULL || elem->circles == NULL ||
                    elem->paths == NULL || elem->groups == NULL ||
                    elem->otherAttributes == NULL ||
                    validateOtherAttributes(elem->otherAttributes) == 0){
                return 0;
            }
        }
        freeNodesInList(groupList);
        free(groupList);
    }

    //2. Contents must represent valid SVG once converted to XML
    //Convert SVG image to XML doc
    xmlDoc* doc = SVGimageToXMLdoc(image);
    
    //validate XML doc
    if(validateXMLtree(doc, schemaFile) == 1){

        xmlFreeDoc(doc);
        xmlCleanupParser();
        return 1;
    }
    xmlFreeDoc(doc);
    xmlCleanupParser();
    printf("xml failed\n");
    return 0;
}


SVGimage* createValidSVGimage(char* fileName, char* schemaFile){
    if(fileName == NULL || schemaFile == NULL) return NULL;

    xmlDoc *doc = NULL;
    SVGimage *newSVG = NULL;

    if (!fileName){
        return NULL;
    }
    
    /*parse the file and get the DOM */
    doc = xmlReadFile(fileName, NULL, 0);

    if (doc == NULL) {
    	return NULL;

    }
    
    if(validateXMLtree(doc, schemaFile) == 0){
        xmlFreeDoc(doc);
        xmlCleanupParser();
        return NULL;
    }
    else{ //xml tree is valid; create SVGimage
        xmlFreeDoc(doc);
        xmlCleanupParser();
        newSVG = createSVGimage(fileName);
    }
    return newSVG;
}

//assumes doc is already validated
bool writeSVGimage(SVGimage* image, char* fileName){
    if(image == NULL || fileName == NULL || strlen(fileName) == 0) return 0;

    int ret = 0;
    
    xmlDocPtr doc = SVGimageToXMLdoc(image);
    
    if(doc){
        ret = xmlSaveFormatFileEnc(fileName, doc, "UTF-8", 1);
    }
    xmlFreeDoc(doc);
    xmlCleanupParser();
    if(ret == -1) return 0;
    return 1;
}




void setAttribute(SVGimage* image, elementType elemType, int elemIndex, Attribute* newAttribute){
    if(image == NULL || newAttribute == NULL) return;
    
    if(elemType == SVG_IMAGE){
        setOtherAttribute(image->otherAttributes, newAttribute);
    }
    else if(elemType == RECT && elemIndex >= 0){
        //Adding or changing an attribute for a rectangle...
        int i = 0;
        ListIterator iter = createIterator(image->rectangles);
        Rectangle* elem = NULL;

        while((elem = (Rectangle*)nextElement(&iter)) != NULL){
            
            if(i == elemIndex){
                char* attr = newAttribute->name;
                if(strcmp(attr, "x")==0){
                    elem->x = atof(newAttribute->value);
                    deleteAttribute(newAttribute);
                }
                else if(strcmp(attr, "y")==0){
                    elem->y = atof(newAttribute->value);
                    deleteAttribute(newAttribute);
                }
                else if(strcmp(attr, "width")==0){
                    elem->width = atof(newAttribute->value);
                    deleteAttribute(newAttribute);
                }
                else if(strcmp(attr, "height")==0){
                    elem->height = atof(newAttribute->value);
                    deleteAttribute(newAttribute);
                }
                else{
                    setOtherAttribute(elem->otherAttributes, newAttribute);
                }
                break;
            }
            ++i;
        }
    }
    else if(elemType == CIRC && elemIndex >= 0){
        int i = 0;
        ListIterator iter = createIterator(image->circles);
        Circle* elem = NULL;
        
        while((elem = (Circle*)nextElement(&iter)) != NULL){
            
            if(i == elemIndex){
                char* attr = newAttribute->name;
                if(strcmp(attr, "cx")==0){
                    elem->cx = atof(newAttribute->value);
                    deleteAttribute(newAttribute);
                }
                else if(strcmp(attr, "cy")==0){
                    elem->cy = atof(newAttribute->value);
                    deleteAttribute(newAttribute);
                }
                else if(strcmp(attr, "r")==0){
                    elem->r = atof(newAttribute->value);
                    deleteAttribute(newAttribute);
                }
                else{
                    setOtherAttribute(elem->otherAttributes, newAttribute);
                }
                break;
            }
            ++i;
        }
        
    }
    else if(elemType == PATH && elemIndex >= 0){
        int i = 0;
        ListIterator iter = createIterator(image->paths);
        Path* elem = NULL;
        
        while((elem = (Path*)nextElement(&iter)) != NULL){
            
            if(i == elemIndex){
                char* attr = newAttribute->name;
                if(strcmp(attr, "d")==0){
                    strcpy(elem->data, newAttribute->value);
                    deleteAttribute(newAttribute);
                }
                else{
                    setOtherAttribute(elem->otherAttributes, newAttribute);
                }
                break;
            }
            ++i;
        }
        
    }
    else if(elemType == GROUP && elemIndex >= 0){
        
        int i = 0;
        ListIterator iter = createIterator(image->groups);
        Group* elem = NULL;
        
        while((elem = (Group*)nextElement(&iter)) != NULL){
            
            if(i == elemIndex){
                //Groups have no necessary attribute. Add to Other
                setOtherAttribute(elem->otherAttributes, newAttribute);
                break;
            }
            ++i;
        }
        
    }
    else return;
}


void addComponent(SVGimage* image, elementType type, void* newElement){
    if(image == NULL || newElement == NULL) return;
    
    if(type == RECT){
        Rectangle* newRect = (Rectangle*)newElement;
        strcpy(newRect->units, "");
        insertBack(image->rectangles, newRect);
    }
    else if(type == CIRC){
        Circle* newCirc = (Circle*)newElement;
        strcpy(newCirc->units, "");
        insertBack(image->circles, newCirc);
    }
    else if(type == PATH){
        Path* newPath = (Path*)newElement;
        insertBack(image->paths, newPath);
    }
    printf("Done adding comp");
}


char* attrToJSON(const Attribute *a){
    if(a == NULL) return "{}";
    
    char* JSONstr = malloc(sizeof(char)*256);
    sprintf(JSONstr, "{\"name\":\"%s\",\"value\":\"%s\"}",
            a->name, a->value);
    return JSONstr;
}


char* circleToJSON(const Circle *c){
    
    char temp[20];
    
    char* JSONstr = malloc(sizeof(char)*256);
    strcpy(JSONstr, "{");
    
    if(c != NULL){
        strcat(JSONstr, "\"cx\":");
        sprintf(temp, "%.2f,", c->cx);
        strcat(JSONstr, temp);
        
        strcat(JSONstr, "\"cy\":");
        sprintf(temp, "%.2f,", c->cy);
        strcat(JSONstr, temp);
        
        strcat(JSONstr, "\"r\":");
        sprintf(temp, "%.2f,", c->r);
        strcat(JSONstr, temp);
        
        strcat(JSONstr, "\"numAttr\":");
        sprintf(temp, "%d,", c->otherAttributes->length);
        strcat(JSONstr, temp);
        
        strcat(JSONstr, "\"units\":\"");
        strcat(JSONstr, c->units);
        strcat(JSONstr, "\"");
    }
    strcat(JSONstr, "}");
    
    return JSONstr;
    
}


char* rectToJSON(const Rectangle *r){
    char temp[20];
    
    char* JSONstr = malloc(sizeof(char)*256);
    strcpy(JSONstr, "{");
    
    if(r != NULL){
        strcat(JSONstr, "\"x\":");
        sprintf(temp, "%.2f,", r->x);
        strcat(JSONstr, temp);
        
        strcat(JSONstr, "\"y\":");
        sprintf(temp, "%.2f,", r->y);
        strcat(JSONstr, temp);
        
        strcat(JSONstr, "\"w\":");
        sprintf(temp, "%.2f,", r->width);
        strcat(JSONstr, temp);
        
        strcat(JSONstr, "\"h\":");
        sprintf(temp, "%.2f,", r->height);
        strcat(JSONstr, temp);
        
        strcat(JSONstr, "\"numAttr\":");
        sprintf(temp, "%d,", r->otherAttributes->length);
        strcat(JSONstr, temp);
        
        strcat(JSONstr, "\"units\":\"");
        strcat(JSONstr, r->units);
        strcat(JSONstr, "\"");
    }
    strcat(JSONstr, "}");
    
    return JSONstr;
    
    
    
}


char* pathToJSON(const Path *p){
    if(p == NULL) return "{}";
    
    char* JSONstr = malloc(sizeof(char)*1024);
    sprintf(JSONstr, "{\"d\":\"%s\",\"numAttr\":%d}",
            p->data, p->otherAttributes->length);
    return JSONstr;
}


char* groupToJSON(const Group *g){
 
    if(g == NULL) return "{}";
    
    int children = g->circles->length+g->rectangles->length+g->paths->length+g->groups->length;
    
    char* JSONstr = malloc(sizeof(char)*256);
    sprintf(JSONstr, "{\"children\":%d,\"numAttr\":%d}",
            children, g->otherAttributes->length);
    
    return JSONstr;
}


char* attrListToJSON(const List *list){

    int i = 0;
    char* JSONstr = malloc(sizeof(char)*512);
    char* oneAttrJSON;
    strcpy(JSONstr, "[");
    
    if(list != NULL && list->length > 0){
        
        ListIterator iter = createIterator((List*)list);
        Attribute* elem = NULL;
        
        //iterates through group list of current group
        while((elem = (Attribute*)nextElement(&iter)) != NULL){
            
            oneAttrJSON = attrToJSON(elem);
            printf("ADDING:  %s\n", oneAttrJSON);
            strcat(JSONstr, oneAttrJSON);
            free(oneAttrJSON);
            
            ++i;
            if(i<list->length){
                strcat(JSONstr, ",");
            }
        }
        
    }
    strcat(JSONstr, "]");
    
    return JSONstr;
}


char* circListToJSON(const List *list){
    int CHUNK = 256;
    int i = 0;
    char* JSONstr = malloc(sizeof(char)*256);
    char* oneCircJSON;
    strcpy(JSONstr, "[");
    
    if(list != NULL){
        
        ListIterator iter = createIterator((List*)list);
        Circle* elem = NULL;
        
        //iterates through group list of current group
        while((elem = (Circle*)nextElement(&iter)) != NULL){
            
            oneCircJSON = circleToJSON(elem);
            strcat(JSONstr, oneCircJSON);
            free(oneCircJSON);
            
            ++i;
            if(i!=list->length){
                strcat(JSONstr, ",");
            }
            JSONstr = realloc(JSONstr, sizeof(char)*(strlen(JSONstr)+CHUNK));
        }
        
    }
    strcat(JSONstr, "]");
    
    return JSONstr;
}


char* rectListToJSON(const List *list){
    int CHUNK = 256;
    int i = 0;
    char* JSONstr = malloc(sizeof(char)*256);
    char* oneRectJSON;
    strcpy(JSONstr, "[");
    
    if(list != NULL){
        
        ListIterator iter = createIterator((List*)list);
        Rectangle* elem = NULL;
        
        //iterates through group list of current group
        while((elem = (Rectangle*)nextElement(&iter)) != NULL){
            
            oneRectJSON = rectToJSON(elem);
            strcat(JSONstr, oneRectJSON);
            free(oneRectJSON);
            
            ++i;
            if(i<list->length){
                strcat(JSONstr, ",");
            }
            JSONstr = realloc(JSONstr, sizeof(char)*(strlen(JSONstr)+CHUNK));
        }
        
    }
    strcat(JSONstr, "]");
    
    return JSONstr;
    
    
}

//!!!!!!!!!!!!!
char* pathListToJSON(const List *list){
    int CHUNK = 1024;
    int i = 0;
    char* JSONstr = malloc(sizeof(char)*1024);
    char* onePathJSON;
    strcpy(JSONstr, "[");
    
    if(list != NULL){
        
        ListIterator iter = createIterator((List*)list);
        Path* elem = NULL;
        
        //iterates through group list of current group
        while((elem = (Path*)nextElement(&iter)) != NULL){
            
            onePathJSON = pathToJSON(elem);
            strcat(JSONstr, onePathJSON);
            free(onePathJSON);
            
            ++i;
            if(i<list->length){
                strcat(JSONstr, ",");
            }
            JSONstr = realloc(JSONstr, sizeof(char)*(strlen(JSONstr)+CHUNK));
        }
        
    }
    strcat(JSONstr, "]");
    
    return JSONstr;
    
    
}

//!!!!!!!!!!!!!
char* groupListToJSON(const List *list){
    int CHUNK = 256;
    int i = 0;
    char* JSONstr = malloc(sizeof(char)*256);
    char* oneGroupJSON;
    strcpy(JSONstr, "[");
    
    if(list != NULL){
        
        ListIterator iter = createIterator((List*)list);
        Group* elem = NULL;
        
        //iterates through group list of current group
        while((elem = (Group*)nextElement(&iter)) != NULL){
            
            oneGroupJSON = groupToJSON(elem);
            strcat(JSONstr, oneGroupJSON);
            free(oneGroupJSON);
            
            ++i;
            if(i<list->length){
                strcat(JSONstr, ",");
            }
            JSONstr = realloc(JSONstr, sizeof(char)*(strlen(JSONstr)+CHUNK));
        }
        
    }
    strcat(JSONstr, "]");
    
    return JSONstr;
    
    
}


char* SVGtoJSON(const SVGimage* imge){
    
    char temp[20];
    
    List* rectList = getRects((SVGimage*)imge);
    List* circList = getCircles((SVGimage*)imge);
    List* pathList = getPaths((SVGimage*)imge);
    List* groupList = getGroups((SVGimage*)imge);
    
    char* JSONstr = malloc(sizeof(char)*128);
    strcpy(JSONstr, "{");
    
    if(imge != NULL){
        strcat(JSONstr, "\"numRect\":");
        sprintf(temp, "%d,", rectList->length);
        strcat(JSONstr, temp);
        
        strcat(JSONstr, "\"numCirc\":");
        sprintf(temp, "%d,", circList->length);
        strcat(JSONstr, temp);
        
        strcat(JSONstr, "\"numPaths\":");
        sprintf(temp, "%d,", pathList->length);
        strcat(JSONstr, temp);
        
        strcat(JSONstr, "\"numGroups\":");
        sprintf(temp, "%d", groupList->length);
        strcat(JSONstr, temp);
        
    }
    strcat(JSONstr, "}");
    
    return JSONstr;
    
}

//---------Helper Functions for A2---------------


xmlDoc* SVGimageToXMLdoc(SVGimage* image){
    if(image == NULL) return NULL;
    
    xmlDocPtr doc = NULL;
    xmlNodePtr root_element = NULL;
    
    doc = xmlNewDoc(BAD_CAST "1.0");
    
    root_element = xmlNewNode(NULL, BAD_CAST "svg");
    
    xmlNsPtr nsPtr = xmlNewNs(root_element, BAD_CAST image->namespace, NULL);
    xmlSetNs(root_element, nsPtr);
    
    xmlDocSetRootElement(doc, root_element);

    if(strlen(image->title) > 0){
        
        xmlNewChild(root_element, NULL, BAD_CAST "title", BAD_CAST image->title);
    }
    if(strlen(image->description) > 0){
        
        xmlNewChild(root_element, NULL, BAD_CAST "desc", BAD_CAST image->description);
    }
    addOtherAttributes(root_element, image->otherAttributes);
    
    //Separate function for convenient recursion. Adds ALL elements to tree.
    addElementsToDoc(image, root_element, NULL);

    return doc;
}


void addElementsToDoc(SVGimage* image, xmlNode* curParent, Group* parentGroup){
    //Loop through rectangles, turning them into xmlNodes, and adding as children
    //Also adds attributes for each new rectangle from list
    List* rectList = NULL;
    List* circleList = NULL;
    List* pathList = NULL;
    List* groupList = NULL;
    
    //printf("Adding children to node with NS: %s\n", (char*)curParent->ns->href);
    
    if(parentGroup!=NULL){ //If the following elements are a group's children..
        rectList = parentGroup->rectangles;
        circleList = parentGroup->circles;
        pathList = parentGroup->paths;
        groupList = parentGroup->groups;
    }
    else{
        rectList = image->rectangles;
        circleList = image->circles;
        pathList = image->paths;
        groupList = image->groups;
    }
    if(rectList->length > 0){
        Rectangle* elem = NULL;
        ListIterator iter = createIterator(rectList);

        //Loop iterates through rectangles, adding their properties and
        //then putting them in list of children of root
        while((elem = (Rectangle*)nextElement(&iter)) != NULL){
            
            xmlNodePtr newRectXML = xmlNewNode(curParent->ns, BAD_CAST "rect");
            char value[10];
            //APPEND UNITS TO THESE VALUES??
            
            //Adding 'x' attribute, first converting to string
            sprintf(value, "%.2f", elem->x);
            strcat(value, elem->units);
            xmlNewProp(newRectXML, BAD_CAST "x", BAD_CAST value);
            //Adding 'y' attribute, first converting to string
            sprintf(value, "%.2f", elem->y);
            strcat(value, elem->units);
            xmlNewProp(newRectXML, BAD_CAST "y", BAD_CAST value);
            //Adding 'width' attribute, first converting to string
            sprintf(value, "%.2f", elem->width);
            strcat(value, elem->units);
            xmlNewProp(newRectXML, BAD_CAST "width", BAD_CAST value);
            //Adding 'height' attribute, first converting to string
            sprintf(value, "%.2f", elem->height);
            strcat(value, elem->units);
            xmlNewProp(newRectXML, BAD_CAST "height", BAD_CAST value);
            
            addOtherAttributes(newRectXML, elem->otherAttributes);
            //Add the new rectangle to root's children
            xmlAddChild(curParent, newRectXML);
        }
    }
    
    //Same as previous, for CIRCLES
    if(circleList->length > 0){
        Circle* elem = NULL;
        ListIterator iter = createIterator(circleList);
        
        while((elem = (Circle*)nextElement(&iter)) != NULL){
            
            xmlNodePtr newCircleXML = xmlNewNode(curParent->ns, BAD_CAST "circle");
            char value[100];
            //APPEND UNITS TO THESE VALUES??
            
            //Adding 'cx' attribute, first converting to string
            sprintf(value, "%.2f", elem->cx);
            strcat(value, elem->units);
            xmlNewProp(newCircleXML, BAD_CAST "cx", BAD_CAST value);
            //Adding 'cy' attribute, first converting to string
            sprintf(value, "%.2f", elem->cy);
            strcat(value, elem->units);
            xmlNewProp(newCircleXML, BAD_CAST "cy", BAD_CAST value);
            //Adding 'r' attribute, first converting to string
            sprintf(value, "%.2f", elem->r);
            strcat(value, elem->units);
            xmlNewProp(newCircleXML, BAD_CAST "r", BAD_CAST value);

            addOtherAttributes(newCircleXML, elem->otherAttributes);
            
            xmlAddChild(curParent, newCircleXML);
        }
    }
    
    //Same as previous, for PATHS
    if(pathList->length > 0){
        Path* elem = NULL;
        ListIterator iter = createIterator(pathList);

        while((elem = (Path*)nextElement(&iter)) != NULL){
            
            xmlNodePtr newPathXML = xmlNewNode(curParent->ns, BAD_CAST "path");

            xmlNewProp(newPathXML, BAD_CAST "d", BAD_CAST elem->data);

            addOtherAttributes(newPathXML, elem->otherAttributes);
            //Add the new rectangle to root's children
            xmlAddChild(curParent, newPathXML);
        }
    }
    
    //For GROUPS, recursively calls assElementsToDoc to add child elements
    
    if(groupList->length > 0){
        Group* elem = NULL;
        ListIterator iter = createIterator(groupList);

        //Loop iterates through rectangles, adding their properties and
        //then putting them in list of children of root
        while((elem = (Group*)nextElement(&iter)) != NULL){
            
            xmlNodePtr newGroupXML = xmlNewNode(curParent->ns, BAD_CAST "g");

            addOtherAttributes(newGroupXML, elem->otherAttributes);
            
            //Add the new group to root's children
            xmlAddChild(curParent, newGroupXML);
            
            addElementsToDoc(image, newGroupXML, elem);
        }
    }
}



void addOtherAttributes(xmlNode* owner, List* attributes){
    
    Attribute* elem = NULL;
    ListIterator iter = createIterator(attributes);

    //Loop iterates through attribute list and adds properties of "owner"
    while((elem = (Attribute*)nextElement(&iter)) != NULL){
        
        xmlNewProp(owner, BAD_CAST elem->name, BAD_CAST elem->value);
    }
}


/* This code is adapted from a post on:
 * http://knol2share.blogspot.com/2009/05/validate-xml-against-xsd-in-c.html
 * by a user named "Sam", posted may 26, 2009
 * I claim no rights to the contents of this function.
 */
bool validateXMLtree(xmlDoc* doc, char* XSDFileName){
    if(XSDFileName == NULL || strlen(XSDFileName) == 0) return 0;
    
    int isValid = 0;

    xmlSchemaPtr schema = NULL;
    xmlSchemaParserCtxtPtr ctxt;

    xmlLineNumbersDefault(1);

    ctxt = xmlSchemaNewParserCtxt(XSDFileName);

    //xmlSchemaSetParserErrors(ctxt, (xmlSchemaValidityErrorFunc) fprintf, (xmlSchemaValidityWarningFunc) fprintf, stderr);
    schema = xmlSchemaParse(ctxt);
    xmlSchemaFreeParserCtxt(ctxt);
    if (doc == NULL){
        return 0;
    }
    else{
        xmlSchemaValidCtxtPtr ctxt;
        int ret;

        ctxt = xmlSchemaNewValidCtxt(schema);
        xmlSchemaSetValidErrors(ctxt, (xmlSchemaValidityErrorFunc) fprintf, (xmlSchemaValidityWarningFunc) fprintf, stderr);
        ret = xmlSchemaValidateDoc(ctxt, doc);
        if (ret == 0){
            isValid = 1;
        }
        else{
            isValid = 0;
        }
        xmlSchemaFreeValidCtxt(ctxt);
    }
    
    if(schema != NULL){
        xmlSchemaFree(schema);
    }
    xmlSchemaCleanupTypes();
    xmlCleanupParser();
    xmlMemoryDump();
    
    return(isValid);
    
}


int setOtherAttribute(List* attrList, Attribute* newAttribute){
    
    if(newAttribute->name == NULL || newAttribute->value == NULL) return 0;
    
    ListIterator iter = createIterator(attrList);
    Attribute* elem = NULL;
    
    while((elem = (Attribute*)nextElement(&iter)) != NULL){
        
        if(strcmp(elem->name, newAttribute->name)==0){
            strcpy(elem->value, newAttribute->value);
            deleteAttribute(newAttribute);
            return 1;
        }
    }
    insertBack(attrList, newAttribute);
    return 0;
}


bool validateOtherAttributes(List* attrs){
    
    ListIterator iter = createIterator(attrs);
    Attribute* elem = NULL;
        
    while((elem = (Attribute*)nextElement(&iter)) != NULL){

        if(elem->name == NULL || elem->value == NULL){
            return 0;
        }
            
    }
    return 1;
}

//----------- FUNCTIONS NEEDED FOR A3 -------------------------------------

SVGimage* JSONtoGPX(const char* svgString){
   
    printf("TEST: %s\n", (char*)svgString);
    
    char str[256];
    char* title;
    char* desc;
    SVGimage * newSVG = NULL;
    
    strcpy(str, (char*)svgString);

    strtok(str,"\"");
    strtok(NULL,"\"");
        
    strtok(NULL,"\"");
    title = strtok(NULL,"\"");
        
    strtok(NULL,"\"");
    strtok(NULL,"\"");
        
    strtok(NULL,"\"");
    desc = strtok(NULL,"\"");
        
    newSVG = initializeSVG();

    strncpy(newSVG->title, title, 256);
    strncpy(newSVG->description, desc, 256);
    strncpy(newSVG->namespace, "http://www.w3.org/2000/svg", 256);
    
    printf("t: %s d: %s ns: %s\n", newSVG->title, newSVG->description, newSVG->namespace);
    
    return newSVG;
     
}

Rectangle* JSONtoRect(const char* svgString){
    
    
    char str[256];
    char* xStr;
    char* yStr;
    char* widthStr;
    char* heightStr;
    char* unitsStr;
    
    Rectangle * newRect = malloc(sizeof(Rectangle));
    
    strcpy(str, (char*)svgString);

    strtok(str,":,");
    xStr = strtok(NULL,":,");
    
    strtok(NULL,":,");
    yStr = strtok(NULL,":,");
    
    strtok(NULL,":,");
    widthStr = strtok(NULL,":,");
    
    strtok(NULL,":,");
    heightStr = strtok(NULL,":,");
    
    strtok(NULL,":,");
    unitsStr = strtok(NULL,":,\"");
    
    newRect->x = strtof(xStr, NULL);
    
    newRect->y = strtof(yStr, NULL);
    newRect->width = strtof(widthStr, NULL);
    newRect->height = strtof(heightStr, NULL);
    
    strncpy(newRect->units, unitsStr, 50);
    
    newRect->otherAttributes = initializeList(attributeToString, deleteAttribute, compareAttributes);
    
    return newRect;
}

Circle* JSONtoCircle(const char* svgString){
    
    printf("TEST: %s\n", (char*)svgString);
    
    char str[256];
    char* cxStr;
    char* cyStr;
    char* rStr;
    char* unitsStr;
    
    Circle * newCircle = malloc(sizeof(Circle));
    
    strcpy(str, (char*)svgString);

    strtok(str,":,");
    cxStr = strtok(NULL,":,");
    
    strtok(NULL,":,");
    cyStr = strtok(NULL,":,");
    
    strtok(NULL,":,");
    rStr = strtok(NULL,":,");
    
    strtok(NULL,":,");
    unitsStr = strtok(NULL,":,\"");
    
    newCircle->cx = strtof(cxStr, NULL);
    newCircle->cy = strtof(cyStr, NULL);
    newCircle->r = strtof(rStr, NULL);
           
    strncpy(newCircle->units, unitsStr, 50);
    
    newCircle->otherAttributes = initializeList(attributeToString, deleteAttribute, compareAttributes);
    
    return newCircle;
}

//------  Additional functions to "glue" C & JS code -----------------

char* svgFileToJSON(char* filePath, char* schemaFile){
    
    char* fileSVGJSON;
    
    printf("getting data from file : %s  with XSD: %s\n", filePath, schemaFile);
    
    SVGimage* newSVG = createValidSVGimage(filePath, schemaFile);
    
    fileSVGJSON = SVGtoJSON(newSVG);
    
    deleteSVGimage(newSVG);
    
    return fileSVGJSON;
    
}


char* svgFileToComponentListJSON(char* filePath, char* schemaFile){
    
    char* compListJSON = malloc(sizeof(char)*4000);
    
    printf("getting Components from file : %s  with XSD: %s\n", filePath, schemaFile);

    
    //Make valid svg struct from file
    SVGimage* newSVG = createValidSVGimage(filePath, schemaFile);
    
    //Connect all component JSON lists into one 2-d JSON array string
    strcpy(compListJSON, "[");
    strcat(compListJSON, rectListToJSON(newSVG->rectangles));
    strcat(compListJSON, ",");
    strcat(compListJSON, circListToJSON(newSVG->circles));
    strcat(compListJSON, ",");
    strcat(compListJSON, pathListToJSON(newSVG->paths));
    strcat(compListJSON, ",");
    strcat(compListJSON, groupListToJSON(newSVG->groups));
    strcat(compListJSON, "]");
    
    deleteSVGimage(newSVG);
    
    return(compListJSON);
    
}


char* svgFileToAttrListJSON(char* filePath, char* schemaFile, char* compDesc){ //compDesc ex/ "Circle 3"
    
    char* attrListJSON = malloc(sizeof(char)*512);
    char* compType;
    char* compNumStr;
    int compNum;
    printf("1\n");
    SVGimage* newSVG = createValidSVGimage(filePath, schemaFile);
    
    //strcpy(attrListJSON, "[{\"name\":\"fill\",\"value\":\"none\"},{\"name\":\"stroke\",\"value\":\"blue\"},"
    //"{\"name\":\"strokewidth\",\"value\":\"1\"}]");
    
    compType = strtok(compDesc, " ");
    compNumStr = strtok(NULL, " ");
    
    compNum = atoi(compNumStr);
    
    List* componentList;
    if(strcmp(compType, "Rectangle")==0){
    	componentList = getRects((SVGimage*)newSVG);
        
    	Node* curNode = componentList->head;
    	for(int i = 0; i<compNum-1; ++i){
            curNode = curNode->next;
    	}
        Rectangle* component = (Rectangle*)curNode->data;
            
        strcpy(attrListJSON, attrListToJSON(component->otherAttributes));
        deleteSVGimage(newSVG);
        return(attrListJSON);
    	
    }else if(strcmp(compType, "Circle")==0){
    	componentList = getCircles((SVGimage*)newSVG);
        
        Node* curNode = componentList->head;
    	for(int i = 0; i<compNum-1; ++i){
            curNode = curNode->next;
    	}
        Circle* component = (Circle*)curNode->data;
        
        strcpy(attrListJSON, attrListToJSON(component->otherAttributes));
        deleteSVGimage(newSVG);
        return(attrListJSON);
        
    }else if(strcmp(compType, "Path")==0){
    	componentList = getPaths((SVGimage*)newSVG);
        
        Node* curNode = componentList->head;
    	for(int i = 0; i<compNum-1; ++i){
            curNode = curNode->next;
    	}
        Path* component = (Path*)curNode->data;
        
        strcpy(attrListJSON, attrListToJSON(component->otherAttributes));
        deleteSVGimage(newSVG);
        return(attrListJSON);
        
    }else if(strcmp(compType, "Group")==0){
    	componentList = getGroups((SVGimage*)newSVG);
        
        Node* curNode = componentList->head;
    	for(int i = 0; i<compNum-1; ++i){
            curNode = curNode->next;
    	}
        Group* component = (Group*)curNode->data;
        
        strcpy(attrListJSON, attrListToJSON(component->otherAttributes));
        deleteSVGimage(newSVG);
        return(attrListJSON);
        
    }else{
    	return NULL;
    }
    
}

char* svgFileToTitleAndDescJSON(char* filePath, char* schemaFile){
    
    char* tit_desc_JSON = malloc(sizeof(char)*512);
    
    SVGimage* newSVG = createValidSVGimage(filePath, schemaFile);
    
    strcpy(tit_desc_JSON, "{\"title\":\"");
    strcat(tit_desc_JSON, newSVG->title);
    strcat(tit_desc_JSON, "\",\"desc\":\"");
    strcat(tit_desc_JSON, newSVG->description);
    strcat(tit_desc_JSON, "\"}");    
    
    return tit_desc_JSON;
}

int changeTitle(char* filePath, char* schemaFile, char* newTitle){
    
    SVGimage* img = createValidSVGimage(filePath, schemaFile);
    
    strncpy(img->title, newTitle, 256);
    
    int status = writeSVGimage(img, filePath);
    
    deleteSVGimage(img);
    
    return status;
}

int changeDesc(char* filePath, char* schemaFile, char* newDesc){
    
    SVGimage* img = createValidSVGimage(filePath, schemaFile);
    
    strncpy(img->description, newDesc, 256);
    
    int status = writeSVGimage(img, filePath);
    
    deleteSVGimage(img);
    
    return status;
}

int editAttrFromSVGFile(char* filePath, char* schemaFile, char* compDesc, char* attrName, char* attrVal){
    SVGimage* img = createValidSVGimage(filePath, schemaFile);
    Attribute* newAttr = malloc(sizeof(Attribute));
    
    newAttr->name = malloc(sizeof(char)*512);
    newAttr->value = malloc(sizeof(char)*512);             //DOUBLE CHECK ATTRIBUTE PARAM SIZES!!!
    
    strcpy(newAttr->name, attrName);
    strcpy(newAttr->value, attrVal);
    
    char* compName = strtok(compDesc, " ");
    
    char* compIndStr = strtok(NULL, " ");
    int compInd = atoi(compIndStr);
    --compInd;
    
    if(strcmp(compName, "Rectangle")==0){
        setAttribute(img, RECT, compInd, newAttr);
    } else if(strcmp(compName, "Circle")==0){
        setAttribute(img, CIRC, compInd, newAttr);
    } else if(strcmp(compName, "Path")==0){
        setAttribute(img, PATH, compInd, newAttr);
    } else if(strcmp(compName, "Group")==0){
        setAttribute(img, GROUP, compInd, newAttr);
    } else if(strcmp(compName, "SVG")==0){
        setAttribute(img, SVG_IMAGE, 0, newAttr);
    } else{
        return 0;
    }
    
    writeSVGimage(img, filePath);
    
    deleteSVGimage(img);
    
    return 1;
}

int createSVGFromScratch(char* filePath, char* schemaFile, char* svgInfoJSON){
    
    SVGimage* img = JSONtoGPX(svgInfoJSON);
    
    if(validateSVGimage(img, schemaFile)){
        writeSVGimage(img, filePath);
        deleteSVGimage(img);
        return 1;
    }
    
    deleteSVGimage(img);
    
    return 0;
}

int addRectToSVG(char* filePath, char* schemaFile, char* rectJSON){
    
    int rectNum;
    SVGimage* img = createValidSVGimage(filePath, schemaFile);
    
    printf("JSON: %s\n", rectJSON);
    
    Rectangle* newRect = JSONtoRect(rectJSON);

    addComponent(img, RECT, newRect);
    
    if(validateSVGimage(img, schemaFile)){
        
        printf("SVG is valid. Writing..\n");
        writeSVGimage(img, filePath);
        rectNum = img->rectangles->length;
        deleteSVGimage(img);
        return rectNum;
    }
    else{
        printf("SVG is NOT valid. Writing..\n");
        deleteSVGimage(img);
        return -1;
    }
}

int addCircToSVG(char* filePath, char* schemaFile, char* circJSON){
    
    int circNum;
    SVGimage* img = createValidSVGimage(filePath, schemaFile);
    
    printf("JSON: %s\n", circJSON);
    
    Circle* newCirc = JSONtoCircle(circJSON);
    //void addComponent(SVGimage* image, elementType type, void* newElement)
    
    addComponent(img, CIRC, newCirc);
    
    if(validateSVGimage(img, schemaFile)){
        
        printf("SVG is valid. Writing..\n");
        writeSVGimage(img, filePath);
        circNum = img->circles->length;
        deleteSVGimage(img);
        return circNum;
    }
    else{
        printf("SVG is NOT valid. Writing..\n");
        deleteSVGimage(img);
        return -1;
    }
}


void scaleRects(char* filePath, char* schemaFile, float factor){
    
    SVGimage* img = createValidSVGimage(filePath, schemaFile);
    
    List* rectList = getRects(img);
    
    ListIterator iter = createIterator(rectList);
    Rectangle* elem = NULL;
    
    while((elem = (Rectangle*)nextElement(&iter)) != NULL){
        elem->width = elem->width*factor;
        elem->height=elem->height*factor;
    }
    if(validateSVGimage(img, schemaFile)){
        writeSVGimage(img, filePath);
    }
    else{
        printf("SVG is invalid after scaling shapes!");
    }
    
    deleteSVGimage(img);
}

void scaleCircs(char* filePath, char* schemaFile, float factor){
    
    SVGimage* img = createValidSVGimage(filePath, schemaFile);
    
    List* circList = getCircles(img);
    
    ListIterator iter = createIterator(circList);
    Circle* elem = NULL;
    
    while((elem = (Circle*)nextElement(&iter)) != NULL){
        printf("old rad=%f", elem->r);
        elem->r = elem->r*factor;
        printf("new rad=%f", elem->r);
    }
    if(validateSVGimage(img, schemaFile)){
        writeSVGimage(img, filePath);
    }
    else{
        printf("SVG is invalid after scaling shapes!");
    }
    
    deleteSVGimage(img);
}