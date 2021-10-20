/*  
 *  ListBuilder.h
 *  Name: Matthew Ruetz
 *  Student #: 0961397
 * 
 */

#ifndef LISTBUILDER_H
#define LISTBUILDER_H
    
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <libxml/parser.h>
#include <libxml/tree.h>
#include <libxml/encoding.h>
#include <libxml/xmlwriter.h>
#include <libxml/xmlschemastypes.h>
#include "SVGParser.h"

/**
 * This method is used to build the SVG image struct by recursively iterating
 * through the values of the XML doc. Recursion was used to more effectively
 * keep track of which group a certain element is in during parsing.
 * 
 * @param a_node - a pointer to the XML node being parsed from. Will be root
 * element on first run, though change as method recurs
 * @param parentGroup - the group in which the current XML element resides
 * @param newSVG - a pointer the the SVGimage struct, does not change with recursion
 * @return - a pointer to an SVG image struct with all information included
 */
SVGimage * build_SVGimage(xmlNode * a_node, Group* parentGroup, SVGimage* newSVG);


/**
 * A simple method used to extract the Title and Description of the SVG image
 * and store these strings in the SVGimage struct.
 * @param svgNode - used to track the current node being read through recursion
 * @param newSVG - pointer to the SVG struct
 */
void getTitleAndDesc(xmlNode* svgNode, SVGimage* newSVG);


/**
 * Allocates memory for an SVGimage struct, and initializes all values
 * @return - pointer to the newly allocated SVGimage struct
 */
SVGimage * initializeSVG();


/**
 * Used to get other attribute information about the SVG image aside from
 * title and description (namespace and other attributes)
 * @param newNode - current node being checked as an attribute through recursion
 * @return - pointer to SVGimage, updated with additional attributes
 */
SVGimage * parseSVG(xmlNode * newNode);


/**
 * Used to initialize and recursively collect all attributes for a Rectangle element and store 
 * in a newly allocated Rectangle struct.
 * @param newNode
 * @return - pointer to new Rectangle struct with all attributes
 */
Rectangle* parseRect(xmlNode * newNode);


/**
 * Used to initialize and recursively collect all attributes for a Circle element and store 
 * in a newly allocated Circle struct.
 * @param newNode
 * @return - pointer to new Circle struct with all attributes
 */
Circle* parseCircle(xmlNode * newNode);


/**
 * Used to initialize and recursively collect all attributes for a Path element and store 
 * in a newly allocated Path struct.
 * @param newNode
 * @return - pointer to new Path struct with all attributes
 */
Path* parsePath(xmlNode * newNode);


/**
 * Used to initialize a Group element, and all of its lists. Also gets attributes
 * of the group for otherAttributes list.
 * @return - pointer to the newly allocated Group struct
 */
Group* initializeGroup();


/**
 * Takes a string, representing an attribute value of an element.
 * Uses isMeasurement() method to see if characters found in string would
 * represent a unit of a measurement attribute.
 * @param val - String representing a measurement value which may have a unit
 * @return - String representing the units of this measurement
 */
char* getUnits(const char* val);


/**
 * Used to initialize and assign name/value of an Attribute struct.
 * @param newNode - an xmlNode representing an attribute
 * @return - pointer to the newly allocated/initialized attribute
 */
Attribute* initializeAttribute(xmlAttr * newNode);


/**
 * Simple method which checks whether an attribute name matches a type of
 * attribute which may have a unit (eg. x, y, width, height, cx, cy, etc.)
 * @param val
 * @return - Boolean: 1 = val is a measurement parameter. 0 = not a measurement
 */
int isMeasurement(char* val);


/**
 * Method to navigate through all groups in the SVG image, including those nested
 * within other groups using recursion to build a list of all Rectangle elements
 * in the entire SVG image. Uses depth-first traversal for nested groups.
 * @param groupList - List of groups within the current parent; either SVG image
 * or a parent group.
 * @param curGroupNode - a List Node pointer to the current group in the list
 * being searched
 * @param rectangleList - A Rectangle list, being added to within this function.
 * This is given as a param to keep data during recursion.
 * @return - a pointer the completed list of all rectangles in the SVG image
 */
List * getRectListFromGroups(List* groupList, Node* curGroupNode, List * rectangleList);


/**
 * Method to navigate through all groups in the SVG image, including those nested
 * within other groups using recursion to build a list of all Circle elements
 * in the entire SVG image. Uses depth-first traversal for nested groups.
 * @param groupList - List of groups within the current parent; either SVG image
 * or a parent group.
 * @param curGroupNode - a List Node pointer to the current group in the list
 * being searched
 * @param circleList - A Circle list, being added to within this function.
 * This is given as a param to keep data during recursion.
 * @return - a pointer the completed list of all circles in the SVG image
 */
List * getCircleListFromGroups(List* groupList, Node* curGroupNode, List * circleList);


/**
 * Method to navigate through all groups in the SVG image, including those nested
 * within other groups using recursion to build a list of all Group elements
 * in the entire SVG image. Uses depth-first traversal for nested groups.
 * @param groupList - List of groups within the current parent; either SVG image
 * or a parent group.
 * @param curGroupNode - a List Node pointer to the current group in the list
 * being searched
 * @param groupList - A Group list, being added to within this function.
 * This is given as a param to keep data during recursion.
 * @return - a pointer the completed list of all groups in the SVG image
 */
List * getGroupListFromGroups(List* groupList, Node* curGroupNode, List* nestedGroupList);


/**
 * Method to navigate through all groups in the SVG image, including those nested
 * within other groups using recursion to build a list of all Path elements
 * in the entire SVG image. Uses depth-first traversal for nested groups.
 * @param groupList - List of groups within the current parent; either SVG image
 * or a parent group.
 * @param curGroupNode - a List Node pointer to the current group in the list
 * being searched
 * @param pathList - A Path list, being added to within this function.
 * This is given as a param to keep data during recursion.
 * @return - a pointer the completed list of all paths in the SVG image
 */
List * getPathListFromGroups(List* groupList, Node* curGroupNode, List * pathList);

//-------------- ASSIGNMENT 2 HELPER FUNCTIONS -----------------------------

/**
 * function for free only the node structs within a list, and leave the data
 * unaltered. Essentially used as a "dummy delete"
 * @param toFreeFrom -List from which all node structures will be free'd.
 */
void freeNodesInList(List* toFreeFrom);


/**
 * This function takes the SVG image as input, and builds an XML tree (doc)
 * by attaching children of the SVG node, then children of groups, and so on
 * in a recursive manner. Once complete, it returns a pointer to the newly
 * allocated xmlDoc.
 * @param image - a pointer to an SVG image, assumed to be valid
 * @return pointer to new xmlDoc
 */
xmlDoc* SVGimageToXMLdoc(SVGimage* image);

/**
 * This function uses code from the tree2.c example found here:
 * http://knol2share.blogspot.com/2009/05/validate-xml-against-xsd-in-c.html
 * It takes an xmlDoc, and XSD (schema) file to which the format of the XML
 * file is compared. It the returns 1 if the XML file is valid, 0 if not.
 * @param doc - pointer to an xmlDoc being validated
 * @param XSDFileName - schema file to compare the doc to and validate
 * @return 1 if valid, 0 if not (bool)
 */
bool validateXMLtree(xmlDoc* doc, char* XSDFileName);

/**
 * This function is used by the SVGimageToXMLdoc function. Once the doc is
 * initialized and, and SVG attributes are added by SVGimageToXMLdoc, this
 * function is called to add all SVG elements to the doc. This is segmented
 * to allow for simple recursion when adding children of groups to the doc.
 * @param image - Pointer to SVG image being translated to doc
 * @param curParent - the current parent node of which elements of this 
 * recursive iteration will be children
 * @param parentGroup - The GROUP that is the current parent node. NULL if
 * current parent is the root node of the doc
 */
void addElementsToDoc(SVGimage* image, xmlNode* curParent, Group* parentGroup);

/**
 * Simple function to iterate through a list of attributes, and associate them
 * as properties of the xmlNode, "owner"
 * @param owner - xmlNode which the properties will be added to
 * @param attributes - list of attributes to associate with the "owner"
 */
void addOtherAttributes(xmlNode* owner, List* attributes);


/**
 * Used by "setAttribute" function to iterate through the otherAttributes list
 * of any element, compare a newAttribute to each. If a matching name is found, the
 * attribute value is update. If no matching name is found, the new attribute
 * is appended to the end of the otherAtributes list.
 * @param attrList - list of attributes to iterate through and change
 * @param newAttribute - new attribute to be added
 * @return 
 */
int setOtherAttribute(List* attrList, Attribute* newAttribute);

/**
 * Simple iterator function to go through a list of attributes, and verify that
 * all attributes do not have a NULL name or value.
 * @param attrs - list of attributes to validate
 * @return 
 */
bool validateOtherAttributes(List* attrs);




#endif /* LISTBUILDER_H */

