# Web Interface Module
Documented By: Abigail MacKersie

Developed By: Abigail MacKersie, Naod Ghebredngl, Prijam Khanal, Sadwal (Luca) Patel, and Tess Gonda

## **0** Introduction
This module will serve as the as the main User Interface for Company C's code project. It will produce an HTML file that will serve as the main visual display of information to the User, and the method through which the User will interact with the program.

## **1** Main Structure

### **1.1** Classes
The WebInterface module will use the Web classes in order to influence and develop HTML code using only C++ through the use of of the Emscripten. The major class that will form the basic structure of the WebInterface is the WebLayout class, which will allow the programmer to edit, delete, and insert new elements into the HTML DOM through C++ code.

### **1.2** Dependencies
The Emscripten library will be used by the WebInterface module from construction, to usage, to deletion. In order to run the module properly, the user will need to have Emscripten downloaded and the ability to access its packages.

The module will also be dependent on several data structures taken from the C++ Standard Library, such as smart pointers, vectors, and maps.

### **1.3** APIs
The module will need information from the Database and Data Analytics teams, as well as from the World and Agents teams. As the main interface module, it will need to have APIs that will bring it all of the information necessary to display for the User.

### **1.4** Build System
The module's necessary build settings and information will be included in the repository Makefile in order to be compatible will all other modules in the system.

## **2** Features
The module will display information in a visual manner with the WebImage class and the WebCanvas class, and provide UI inputs such as buttons, textboxes, radio buttons, and sliders to change the settings on the simulation. These features will be in accordance with the general vision for the look and feel of the game's content and interface.

There will be two main layouts: one for the Overworld and one for the Underworld, both of which will be visually distinct from one another. The simulation will be displayed dynamically for the User and provide visual representations of the Agents moving throughout the World grids. 

Those layouts will be viewable through the Emscripten HTML compilation, which will allow the User to host the game in a local web host browser.

## **3** Display Structure
For the current version of the game, the gameplay will take place through two stages. The first stage is the Settings Stage, during which the User will change the settings of the simulation and select the behavior patterns they want the AI Agents to follow. Once the settings have been confirmed by the User, the game will transition into the Simulation Stage. During the Simulation Stage, the User will no longer have control over the settings of the simulation and will simply watch the visual representation of the civilization running its course.

If a multiplayer combat system is introduced, a third stage will be added that allows the User to watch a simulated battle between the results of their civilization and the results of another player's civilization.