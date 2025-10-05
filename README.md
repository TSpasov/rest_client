A \*\*C++17\*\* client for the \*\*Progress MOVEit Transfer REST API\*\*, demonstrating:



\- Clear separation of concerns between CLI, business logic, and GUI  

\- Modern CMake build targeting \*\*Windows / Visual Studio 2022\*\*  

\- Interfaces and dependency injection (`IHttpClient`, `ILogger`)  

\- RAII resource handling for libcurl  

\- SOLID design principles in a practical, compact implementation  



The project includes \*\*two front-ends\*\* — a \*\*CLI app\*\* and a \*\*Win32 GUI app\*\* —



**Structure:**



rest\_client/

├── CMakeLists.txt

│

├── cli/

│ └── main.cpp # CLI entry point

│

├── gui/

│ ├── WinMain.cpp # Win32 GUI entry point

│ ├── app.rc, resource.h # Dialog \& control definitions

│

├── include/

│ ├── common/HttpCommon.hpp # HTTP codes, constants, enums

│ ├── net/ # IHttpClient, CurlHttpClient, RetryingHttpClient

│ ├── models/ # Token, User, UploadResult

│ ├── services/ # AuthService, UserService, UploadService

│ └── Logger.hpp # Unified logging (stdout, file, GUI)

│

├── src/

│ ├── net/ # HTTP client implementations

│ └── services/ # Business logic implementations

│

└── build/ # CMake build output

├── moveit\_cli.exe

└── moveit\_gui.exe







**Install dependencies:**


git clone https://github.com/microsoft/vcpkg.git C:\\dev\\vcpkg

C:\\dev\\vcpkg\\bootstrap-vcpkg.bat

C:\\dev\\vcpkg\\vcpkg.exe install curl:x64-windows





C:\\dev\\vcpkg\\vcpkg.exe integrate install



**Configure:**


mkdir build

cd build



cmake -G "Visual Studio 17 2022" -A x64 .. ^

&nbsp; -DCMAKE\_TOOLCHAIN\_FILE=C:\\dev\\vcpkg\\scripts\\buildsystems\\vcpkg.cmake ^

&nbsp; -DVCPKG\_TARGET\_TRIPLET=x64-windows



**Build**:



Build both targets:

cmake --build . --config Release



or separately:

cmake --build . --config Release --target moveit\_cli

cmake --build . --config Release --target gui\_cli


**Run:**

.\\Release\\moveit\_cli.exe https://testserver.moveitcloud.com username password path\\to\\file.txt
.\\Release\\moveit\_gui.exe




**Architecture**:

            +--------------------+

&nbsp;           |      CLI App       |

&nbsp;           |  (args, stdout)    |

&nbsp;           +---------+----------+

&nbsp;                     |

&nbsp;                     v

+----------------------------------------------+

|               Business Logic                 |

|----------------------------------------------|

|  Services:                                   |

|   • AuthService      → Handles authentication |

|   • UserService      → Fetches user data      |

|   • UploadService    → Uploads/deletes files  |

|                                              |

|  Infrastructure:                             |

|   • net/: CurlHttpClient, RetryingHttpClient  |

|   • common/: HttpCommon, Logger               |

|                                              |

|  Models/DTOs:                                |

|   • Token, User, UploadResult, Error          |

|                                              |

|  Interfaces (Ports):                         |

|   • IHttpClient, ILogger                      |

+-------------------+--------------------------+

&nbsp;                   ^

&nbsp;                   |

&nbsp;           +-------+--------+

&nbsp;           |      GUI       |

&nbsp;           | (Win32 native) |

&nbsp;           +----------------+









