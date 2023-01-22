# Trokam

## What is Trokam?

Trokam is an internet search engine. These are some of the unique features of Trokam:

1. **Ad-free**. No advertisement at all!
2. **Transparency**. You know how results are ranked.
3. **Free software**. It is built entirely on open-source software.
4. **Privacy**. Your searches are private.

You can try it now [Trokam](http://trokam.com/).

## How does it work?

Trokam works with three binaries: the web server, the crawler and the pump.

The web server interacts with the user and queries the databases. The crawler indexes pages and feed a database. The web server and crawler databases are generally different and located on different machines. Both operate independently.

The pump's task is to run the crawler and copy the database to the web server incrementally and cyclically. After some time specified in a configuration file, the pump will delete the crawler's database and start the process again, first indexing a set of seed URLs.

## Operating System

Trokam has been tested exclusively on Ubuntu Linux Server 20.04.5.
These instructions assume you are working on the same operating system.
Building and running on any mature Unix-like operating system should be possible with minor modifications.

## License

Trokam code is licensed as the GNU General Public License v3.0 (GPLv3). The file [LICENSE](https://github.com/trokam/step3.trokam.com/blob/master/LICENSE) is distributed with the code. Read [A Quick Guide to GPLv3](https://www.gnu.org/licenses/quick-guide-gplv3.html) to learn the essentials of this license.

## Building Trokam

### Sources organization


### The webserver

#### Prerrequisites



#### Build the webserver


### The data pump


#### Prerrequisites


#### Build the data pump



Build Trokam binaries following these commands,

    $ mkdir build
    $ cd build
    $ cmake ..
    $ make



