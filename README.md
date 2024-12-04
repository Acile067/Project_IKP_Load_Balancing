# Load Balancing

This project was developed as part of a course at the Faculty of Technical Sciences in Novi Sad, for the subject Industrial Communication Protocols.

The task was to develop a service for storing data received from clients. The service consisted of one Load Balancer (LB) component and an arbitrary number (N) of Worker (WR) components.

## Built using 

- &nbsp;<img src="https://upload.wikimedia.org/wikipedia/commons/3/35/The_C_Programming_Language_logo.svg" align="center" width="28" height="28"/> <a href="https://en.wikipedia.org/wiki/C_(programming_language)"></a>
- &nbsp;<img src="https://upload.wikimedia.org/wikipedia/commons/1/18/ISO_C%2B%2B_Logo.svg" align="center" width="28" height="28"/> <a href="https://en.wikipedia.org/wiki/C%2B%2B"></a>

## Installation:

### Install a C++ Compiler

Ensure you have a C++ compiler installed. Supported compilers include:  

- **[GCC](https://gcc.gnu.org/)**: Available for Linux, macOS, and Windows (via MinGW).  
- **[MSVC](https://visualstudio.microsoft.com/)**: Provided with Visual Studio on Windows.  
- **[Clang](https://clang.llvm.org/)**: Compatible with Linux, macOS, and Windows.  

Follow the respective links for download and installation instructions based on your operating system.  

## How to run (Windows):

1. **Clone the project**:

   Clone the repository and navigate to the Load Balancer project directory:
   
   ```bash
   git clone https://github.com/Acile067/Project_IKP_Load_Balancing.git
   cd Project_IKP_Load_Balancing/Project_IKP_Load_Balancer
   ```

3. **Build using MSVC**:

    Open the Developer Command Prompt for VS 2022 and build the project using the following command:
  
   ```bash
   msbuild Project_IKP_Load_Balancer.sln /p:Configuration=Debug /p:Platform=x64
   ```

3. **Run the projec**:

    To start the Load Balancer and initialize the required components (Workers and Clients), simply double-click on:

   ```run.bat```
