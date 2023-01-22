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

The web server interacts with the user and queries the page databases. The crawler indexes pages and feed a page database. The web server and crawler page databases are generally different and located on different machines. Both operate independently.

The pump's task is to run the crawler and copy the page database to the web server incrementally and cyclically. After some time specified in a configuration file, the pump will delete the crawler's page database and start the process again, first indexing a set of seed URLs.

## Operating System

Trokam has been tested exclusively on Ubuntu Linux Server 20.04.5.
These instructions assume you are working on the same operating system.
Building and running on any mature Unix-like operating system should be possible with minor modifications.

## Programming Language

Trokam is entirely programmed in C++.

## License

Trokam code is licensed as the GNU General Public License v3.0 (GPLv3). The file [LICENSE](https://github.com/trokam/step3.trokam.com/blob/master/LICENSE) is distributed with the code. Read [A Quick Guide to GPLv3](https://www.gnu.org/licenses/quick-guide-gplv3.html) to understand the essentials of this license.

## Building Trokam

### Sources organization

There are directories associated with each one of the binaries and one for the common files. The directory `include` keeps all the headers. Under the directory `etc` you find configuration files.

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

### The web server

#### Prerequisites

Trokam web server depends on several libraries. Most of these libraries are available to install using the package manager. Hence start installing them,

    $ sudo aptitude install g++ cmake libboost1.67-all-dev libapache2-mod-fcgid apache2 postgresql libpqxx-dev libfcgi-dev libfcgi-bin libfmt-dev nlohmann-json3-dev libxapian-dev

Download the latest sources of the library [Wt](https://www.webtoolkit.eu/wt/download). This one is unavailable to install with the package manager, so you must build and install it from its sources. Once you have decompressed, move inside its root directory and execute,

    $ mkdir build
    $ cd build
    $ cmake -DCMAKE_INSTALL_PREFIX=/usr/local ..
    $ make
    $ sudo make install
    $ sudo ldconfig

#### Configure PostgreSQL

The web server uses three types of databases. The page databases store the content being searched by users. They are implemented with [Xapian](https://xapian.org/). There could be any number of them.

Also, two [PostgreSQL](https://www.postgresql.org/) databases: `transfers` and `events`. One instance of each one.

The `transfer` database tells the web server which page databases are enabled and where they are located to perform the search. The web server generally has several page databases to perform the query, but some of them may be disabled temporarily while the `pump` updates it.

The `events` database saves the timestamp every time a user does a new search. It is used to know the server usage.

Postgresql allows quite flexible settings, but these instructions will explain the most basic one: `transfers` and `events` will be in the same machine as the web server, and access will be allowed to every machine in the same private network.

    $ sudo su - postgres
    [postgres] $ createuser web_user
    [postgres] $ createdb transfers
    [postgres] $ createdb events
    [postgres] $ cd /etc/postgresql/12/main/

Edit the file `pg_hba.conf`. Replace the line:

    local   all             all                             peer

by this one:

    local   all             all                             trust

And replace this line:

    host    all             all         127.0.0.1/32        md5

by this one:

    host    all             all         aaa.bbb.0.0/24      trust

`aaa.bbb.0.0` is a IP mask of the private network.

Also edit the file `postgresql.conf`. Replace this line:

    #listen_addresses = 'localhost'		# what IP address(es) to listen on;

With this one:

    listen_addresses = '*'			# what IP address(es) to listen on;

If you are just getting acquainted with Trokam's code and testing it with virtual machines on a desktop computer, the web server, the crawlers, and the host will be in the same network. There is no distinction between private and public networks. And it does not matter in this case.

However, if you are setting Trokam in the cloud or your organisation's intranet, ensure that the IP mask corresponds to the private network used by the web server and crawlers, not the public network.

Accordingly, the web server should have at least two IPs, one that belongs to the public network and another to the private network. The private IP should be like aaa.bbb.xxx.yyy.

[PostgreSQL security settings](https://www.postgresql.org/docs/12/auth-pg-hba-conf.html) allow you several other possibilities. Read the documentation and feel confident with these settings. Learning about PostgreSQL is time well spent.

Finally,

    [postgres] $ exit
    $ sudo systemctl restart postgresql
    $ psql -U web_user transfers < transfers.postgresql
    $ psql -U web_user events < events.postgresql

#### Configure Apache



#### Build the web server


### The crawler and data pump


#### Prerequisites


#### Build the data pump
