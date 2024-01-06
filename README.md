

<!-- PROJECT LOGO -->
<br />
<div align="center">
  <a href="(https://github.com/thisisamna/OS_Project)">
    <img src="images/logo.png" alt="Logo" width="80" height="80">
  </a>

  <h3 align="center">FOS</h3>

  <p align="center">
FOS, our mini operating system!
       <br />
    <a href="https://github.com/othneildrew/Best-README-Template"><strong>Explore the docs »</strong></a>
    <br />
    <br />
    <a href="https://github.com/othneildrew/Best-README-Template">View Demo</a>
    ·
    <a href="https://github.com/othneildrew/Best-README-Template/issues">Report Bug</a>
    ·
    <a href="https://github.com/othneildrew/Best-README-Template/issues">Request Feature</a>
  </p>
</div>



<!-- TABLE OF CONTENTS -->
<details>
  <summary>Table of Contents</summary>
  <ol>
    <li>
      <a href="#about-the-project">About The Project</a>
      <ul>
        <li><a href="#team">The Team</a></li>
      </ul>
    </li>
    <li>
      <a href="#Modules">Modules</a>
      <ul>
        <li><a href="#system-calls">System calls</a></li>
        <li><a href="#memory-allocation"> Memory allocation</a></li>        
        <li><a href="#fault-handler">Fault Handler</a></li>
        <li><a href="#cpu-scheduling"> CPU Scheduling</a></li>
   </ul>
    </li>
    <li><a href="#usage">Usage</a></li>
    <li><a href="#roadmap">Roadmap</a></li>
    <li><a href="#contributing">Contributing</a></li>
    <li><a href="#license">License</a></li>
    <li><a href="#contact">Contact</a></li>
    <li><a href="#acknowledgments">Acknowledgments</a></li>
  </ol>
</details>



<!-- ABOUT THE PROJECT -->
## About The Project

[![FOS Screen Shot][fos-screenshot]](https://example.com)

This is OS'23 course project at FCIS, Ain Shams University, better known as FOS.
* Supervisor and course lecturer: Dr. Ahmed Salah
* Mentor TA: Dr Rezq Mohammed

  
### Team:
* Amna Ahmed - [@thisisamna](https://github.com/thisisamna) 
* Khira Ebe - [@monaya37](https://github.com/monaya37) 
* Mariam Reda - [@Mariam428](https://github.com/Mariam428) 
* Fatma Gamal - [@FatmaGG](https://github.com/FatmaGG) 
* Merna Tareq
* Manar Mansy - [@manarmansyy](https://github.com/manarmansyy) 


### Built With
* C


<!-- GETTING STARTED -->
## Modules

### System Calls

- System call implementation
- Linking user side and kernel side functions

### Memory Allocation

Managing user and kernel memory using two schemes
- Dynamic (block) allocator
	- Allocation using first fit and best fit strategies
	- Freeing allocated blocks
	- Reallocation using first fit strategy
- Page allocator
	-  Allocating pages 
	- Freeing memory pages
	- Lazy allocation/deallocation in the case of user memory
### Fault Handler
Validate faulted addresses. If valid, handle the fault either through placement and replacement, using one of two schemes:
- FIFO (First in first out)
- LRU (Least recently used)

### CPU Scheduler
- BSD scheduler implementation
-  Environment (process) management
- Clock interrupt handler



<!-- ROADMAP -->
## Known issues

- [ ] Process command
- [ ] Sbrk (negative decrement) and sys_sbrk 
- [ ] Freeing user memory 

<!-- ACKNOWLEDGMENTS -->
## Acknowledgments

Thanks to Dr. Ahmed Salah, Dr. Rezq Mohamed and all TAs who supported this project. 

Your help was invaluable to our team and every team. :)

