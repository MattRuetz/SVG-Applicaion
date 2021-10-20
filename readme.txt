SVG web application

Matthew Ruetz

This is a wed application using Node.js framework, originally created for the CIS2750 program at the University of Guelph.

The purpose of this application is to:
a) Give users the ability to access a library of SVG images (stored in the "uploads" folder [database])
b) Allow users to manipulate features (rectangles, circles, paths, and whole groups) by scaling, translating, and altering the color of these elements.
c) Allow for the storage of altered / custom SVG images created using the tools within the web app
d) Upload their own SVG images to be stored in the local database


---- CONTENTS ----

/parser : contains C code which provides all data manipulation and organization functions.
	/parser/src : contains source code
	/parser/include : contains header files

/public : contains JS, CSS and HTML (index.html, index.js, style.css) to run the web app with Node.js


