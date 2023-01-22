# Trokam

## What is Trokam?

Trokam is an internet search engine. These are some of the unique features of Trokam:

1. **Ad-free**. No advertisement at all.
2. **Transparency**. You know how results are ranked.
3. **Free software**. It is built entirely on open-source software.
4. **Privacy**. Your searches are private.

You can try it now [Trokam](http://trokam.com/).

## How does it Work?

Trokam works with three binaries: the web server, the crawler and the pump.

The web server interacts with the user and queries the databases. The crawler indexes pages and feed a database. The web server and crawler databases are generally different and located on different machines. Both operate independently.

The pump's task is to run the crawler and copy the database to the web server incrementally and cyclically. After some time specified in a configuration file, the pump will delete the crawler's database and start the process again, first indexing a set of seed URLs.

## Operating System

Trokam has been tested exclusively on Ubuntu Linux Server 20.04.5.
These instructions assume you are working on the same operating system.
Building and running on any mature Unix-like operating system should be possible with minor modifications.

## Programming Language

Trokam is entirely programmed in C++.

## License

Trokam code is licensed as the GNU General Public License v3.0 (GPLv3). The file [LICENSE](https://github.com/trokam/step3.trokam.com/blob/master/LICENSE) is distributed with the code. Read [A Quick Guide to GPLv3](https://www.gnu.org/licenses/quick-guide-gplv3.html) to learn the essentials of this license.

## Building Trokam

### Sources organization

There are directories associated with each one of the binaries and one for the common files. The directory 'include' keeps all the headers.

    step3.trokam.com
    ├── CMakeLists.txt
    ├── LICENSE
    ├── README.md
    ├── etc
    ├── include
    ├── src_common
    ├── src_crawler
    ├── src_pump
    └── src_web

### The webserver

#### Prerequisites

Trokam web server depends on several libraries. Most of these libraries are available to install using the package manager installer. Hence start installing them,

    sudo aptitude install g++ cmake libboost1.67-all-dev libapache2-mod-fcgid apache2 postgresql libpqxx-dev libfcgi-dev libfcgi-bin libfmt-dev nlohmann-json3-dev libxapian-dev

Download the latest sources of the library [Wt](https://www.webtoolkit.eu/wt/download). This one is unavailable to install with the package manager, so you must build and install it from its sources. Once you have decompressed, move inside its root directory and execute,

    cmake -DCMAKE_INSTALL_PREFIX=/usr/local ..
    make -j2
    sudo make install
    sudo ldconfig

#### Configure PostgreSQL


#### Configure Apache


#### Build the webserver


### The crawler and data pump


#### Prerrequisites


#### Build the data pump
