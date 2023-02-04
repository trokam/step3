# Trokam

## What is Trokam?

Trokam is an internet search engine. These are some of the unique features of Trokam:

1. **Funding**. It is entirely funded by [donations](https://trokam.com/info/donate). Trokam is ad-free.
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

## Support Building Trokam

If you find a mistake or something that needs to be added to these instructions, possibly impeding you from successfully building Trokam, please get in touch with the project developer, Nicolas Slusarenko, at nicolas.slusarenko@trokam.com.

## Building and Installing Trokam

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

    host    all             all         aaa.bbb.ccc.0/24    trust

`aaa.bbb.ccc.0` is a IP mask of the private network.

Also edit the file `postgresql.conf`. Replace this line:

    #listen_addresses = 'localhost'        # what IP address(es) to listen on;

With this one:

    listen_addresses = '*'            # what IP address(es) to listen on;

If you are just getting acquainted with Trokam's code and testing it with virtual machines on a desktop computer, the web server, the crawlers, and the host will be in the same network. There is no distinction between private and public networks. And it does not matter in this case.

However, if you are setting Trokam in the cloud or your organisation's intranet, ensure that the IP mask corresponds to the private network used by the web server and crawlers, not the public network.

Accordingly, the web server should have at least two IPs, one that belongs to the public network and another to the private network. The private IP should be like aaa.bbb.ccc.ddd.

[PostgreSQL security settings](https://www.postgresql.org/docs/12/auth-pg-hba-conf.html) allow you several other possibilities. Read the documentation and feel confident with these settings. Learning about PostgreSQL is time well spent.

Restart PostgreSQL engine,

    [postgres] $ exit
    $ sudo systemctl restart postgresql

Create the database schema,

    $ psql -U web_user transfers < transfers.postgresql
    $ psql -U web_user events < events.postgresql

Finally, test the access to these databases from the web server. Here is the example for `transfers`,

    $ psql -U web_user transfers
    psql (12.12 (Ubuntu 12.12-0ubuntu0.20.04.1))
    Type "help" for help.

    transfers=> \dt
            List of relations
    Schema |    Name     | Type  |  Owner
    --------+-------------+-------+---------
    public | dbcontent   | table | web_user
    (1 row)
    transfers=> \q

Finally, test the access to these databases from the crawler machine. The command must use the private IP address of the web server. Here is the example for `transfers`,

    $ psql -U web_user -h aaa.bbb.ccc.ddd transfers
    psql (12.12 (Ubuntu 12.12-0ubuntu0.20.04.1))
    SSL connection (protocol: TLSv1.3, cipher: TLS_AES_256_GCM_SHA384, bits: 256, compression: off)
    Type "help" for help.

    transfers=> \dt
            List of relations
    Schema |    Name     | Type  |  Owner
    --------+-------------+-------+---------
    public | dbcontent   | table | web_user
    (1 row)
    transfers=> \q

#### Configure Apache

Trokam web server is a binary executed by Apache as a fast CGI. So start adding the Trokam's site to the Apache configuration and enable it. And disable the default site.

    sudo cp trokam_web_http.conf /etc/apache2/sites-available/
    cd /etc/apache2/sites-available/
    sudo a2ensite trokam_web_http.conf

    cd /etc/apache2/sites-available/
    sudo a2dissite 000-default.conf

Modify Apache's fast CGI configuration. In the file /etc/apache2/mods-available/fcgid.conf replace this line,

    AddHandler fcgid-script .wt

By this one,

    AddHandler fcgid-script .fcgi

Finally, enable the CGI module and reload Apache,

    sudo a2enmod fcgid
    sudo systemctl reload apache2

#### Configure Wt

Go to directory `/etc/wt` and edit the file `wt_config.xml`. Replace this line:

    <progressive-bootstrap>false</progressive-bootstrap>

by this one,

    <progressive-bootstrap>true</progressive-bootstrap>

Add this line just before the closing tag `</properties>`:

    <property name="approot">/usr/local/etc/trokam/approot</property>

Create the directory `wt` and set its ownership to `www-data`:

    $ sudo mkdir -p /var/run/wt
    $ chown www-data:www-data -R /var/run/wt/

Be sure that this directory will be there after a reboot. Hence, log in as root and edit the cron configuration,

    # crontab -e

Then add this line and save the configuration,

    @reboot mkdir -p /var/run/wt; chown www-data:www-data -R /var/run/wt/

#### Build and Install the Web Server

The web server is build using CMake, following the usual procedure:

    git clone git@github.com:trokam/step3.trokam.com.git
    cd step3.trokam.com/
    mkdir build_web
    cmake -DSUBSYSTEM=WEB ..
    make
    sudo make install

Take a look a the output. The last command installs binaries and files with specific settings.

#### Testing the Web Server

Use a browser and point to the public IP address of the web server. You should get the Trokam start page. If you search for a term, you won't have any results because there are no databases, but this is not an error.

The file,

    /var/log/apache2/access.log

tells you what pages are accessed on the server. And this file,

    /var/log/apache2/error.log

provides valuable information for debugging. Use these files as the primary source of information on what is happening with the server.

### The Crawler and the Pump

#### Prerequisites

Install the libraries available with the package manager,

    $ sudo aptitude install cmake g++ libcurl4-openssl-dev libpqxx-dev postgresql postgresql-client-common libxapian-dev xapian-tools libboost1.67-all-dev libexttextcat-dev libexttextcat-data nlohmann-json3-dev libfmt-dev libncurses-dev sshfs

Trokam uses a modified version of the Lynx browser to convert the HTML pages to text files. Download it and build it. Do not install it using make, but copy the files as indicated,

    $ git clone https://github.com/trokam/lynx_modified
    $ cd lynx_modified/
    $ ./configure
    $ make
    $ sudo cp ./lynx /usr/local/bin/lynx_mod
    $ sudo cp ./lynx.cfg /usr/local/etc/lynx_mod.cfg

#### Configure PostgreSQL

    $ sudo su - postgres
    [postgres] $ createuser crawler_user
    [postgres] $ createdb warehouse
    [postgres] $ cd /etc/postgresql/12/main/

Edit the file `pg_hba.conf`. Replace the line:

    local   all             all                             peer

by this one:

    local   all             all                             trust

Restart PostgreSQL engine,

    [postgres] $ exit
    $ sudo systemctl restart postgresql

Create the database schema,

    $ psql -U crawler_user warehouse < warehouse.postgresql

#### Building

Now, you have all set to compile and install the crawler and the pump. Build the pump,

    $ mkdir build_pump
    $ cd build_pump/
    $ cmake -DSUBSYSTEM=PUMP ..
    $ make
    $ sudo make install

Change to the root directory of your source files and build the crawler,

    $ mkdir build_crawler
    $ cd build_crawler/
    $ cmake -DSUBSYSTEM=CRAWLER ..
    $ make
    $ sudo make install

#### Running the Pump

Execute the pump and save its output for review its operation,

    $ pump 2>&1 1>>/tmp/pump.log &
    $ exit

