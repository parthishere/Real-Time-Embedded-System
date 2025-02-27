# simple project with EK-TM4C1294XL LaunchPad



## Getting Started

These instructions will give details on how to create simple project using TivaWare library and code composer studio to Initialize and use GPIO in EK-TM4C1294XL LaunchPad

### Prerequisites

What software you need to install 

```
 1- Download and start the latest version of Code Composer Studio (CCS)  from http://processors.wiki.ti.com/index.php/Download_CCS 
 while installation select the processors that your CCS installation will support. Select at least 32-bit ARM MCUs in order to run our project
```
```
2-Download and install the latest full version of TivaWare from: http://www.ti.com/tool/sw-tm4c 
 If at all possible, please install TivaWare into the default folder named C:\TI\TivaWare_C_Series-2.x.x.xxxxx 
```

### kit initialization
```
► JP1 selects where the LaunchPad will be connected to power. The choices are power from ICDI (debug USB port), OTG (user USB port) or BoosterPack . Make sure the jumper is in the ICDI position 
► JP2 is a power measurement point for MCU current only.  Make sure this jumper is in place
► JP3 is a power measurement point for the entire LaunchPad board’s current. Make sure this jumper is in place
► JP4 and 5 configure the LaunchPad for either CAN or UART communication. Vertical is CAN and horizontal is UART . Make sure that all four jumpers are in the horizontal (UART) position   
```


## initialize code composer studio

 Create a New Project 
```
► select Project → New CCS Project:   
► In the New CCS Project dialog, select Tiva C Series as the target and Tiva TM4C1294NCPDT for the part. Be careful making this selection. 
► For the Connection, pick Stellaris In-Circuit Debug Interface. This is the built-in emulator on the LaunchPad board. 
► Name the project test-gpio 
► Uncheck the Use default location checkbox and browse to the target location in your pc 
► In the Project templates and examples box, pick Empty Project (with main.c) and click Finish.
```
- In the Project Explorer pane, click on the  to the left of test-gpio to expand the project. Note that main.c is already open for editing in the Editor pane.   
### adding  TivaWare driverlib.lib file to project 
```
Users can ADD (copy or link) files into their project by
 • SOURCE files are typically COPIED
 • LIBRARY files are typically LINKED (referenced):
      - They are located outside the project folder via : • relative path using exist variables like  PROJECT_LOC     • absolute path using build variables to make the project portable
```
```
build variables
Code Composer allows the use of a vars.ini file to define workspace variables and a macros.ini file to define project variables.
► create text file to contain the variable name refer to the path of tivaware folder
► inside the file type the variable name and path as follow: TIVAWARE_INSTALL = c:\TI\TivaWare_C_Series-2.1.4.178
► save the file as vars.ini in your directory folder
► Right-click on project name in the Project Explorer pane of CCS. Select Import, and then Import … In the next dialog, expand Code Composer Studio. 
► Select Build Variables and click Next.  
► In the next dialog (shown below), browse to the file you created as vars.ini 
- Now you can use this variable for the paths that CCS will need to find your files. If, at a later date, you update TivaWare and it has a new folder name, the only edit you need to make is here in vars.ini. If you change workspaces, you will have to re-import vars.ini.    
 
 
Link driverlib.lib to Your Project 
► Select Project then Add Files… Navigate to:  C:\TI\TivaWare_C_Series-2.1.0.12573 \driverlib\ccs\Debug\driverlib.lib then click open  
► select the option of link to files and choose the variable name of TIVAWARE_INSTALL which created before 




Add the INCLUDE search paths for the header files 

►  Right-click on your test-gpio project in the Project Explorer pane and select Properties.
► Click on Build → ARM Compiler → Include Options
► In the Add dir to #include search path pane, click the “+” sign next to Add dir to #include search path  (   
► add the build variable you created earlier 
```
## Sponsored by
<a href = "https://the-diy-life.co">
<img src="https://the-diy-life.co/images/logo_diylife.jpg"  width="248" height="248">
</a>


