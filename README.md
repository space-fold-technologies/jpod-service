<!-- ABOUT THE PROJECT -->
## JPOD DAEMON
JPOD is container engine capable of running both `FreeBSD-Jails` and `LinuxJails` in a similar fashion to (Docker) containers.


Here's why:
* I like FreeBSD for a lot of personal reasons and wanted to write something to let me use it for all sorts of development in my off hours
* I figured this might be a good way to get to know a little about how some stuff really works outside my day job which is not
* Good to learn and keep learning C and C++ (I'm terrible at it but, I don't want to give up)

At the very end, I hope to slowly create something with most of the `Docker` features I commonly use and keep cleaning up the code so that it becomes clearer to all those who read it, especially me after 3 months.


<!-- SET UP -->
## Setup

This covers the set for the daemon and client for both development and testing

### Prerequisites

To enable the development and testing jpod the following dependencies and configurations need to be setup.
* enable jails, linux emulation, file systems and networking support in rc.conf
  ```sh
  # __ KERNEL MODULES LOAD __
    kld_list="linux linux64 fdescfs linprocfs linsysfs tmpfs"

  # __ JAIL SETUP __
    jail_enable="YES"
    jail_parallel_start="YES"  
  ```

* enable packet forwarding
  ```sh
    sysctl net.inet.ip.forwarding=1
  ```

* enable virtual networking support, bridge, epair and pf[packet filter] in rc.conf
  ```sh
  # __ VNET SUPPORT FOR JAILS __
    if_bridge_enable="YES"
    if_epair_enable="YES"
    pf_enable="YES"
  ```

* install development tools needed on FreeBSD [13.2 -> 14]
  ```sh
    sudo pkg update && sudo pkg install cmake ninja sqlite3
  ```

* install conan 1.62.0 [13.2 -> 14] to handle dependency management of the project
  ```sh
    pip install --user conan
  ```

* clone the client and server source code to your project directories_

  ```sh
    git clone git@github.com:space-fold-technologies/jpod-service.git
  ```

### Development Configuration
  
_This part is mean to allow for smooth development without having to move out of the project folder
 inside the project folder exists the settings file `resource/settings.yml`_ 
1. Open the yaml file and substitute the paths with the destinations of your choice
   ```yaml
   containers:
    path: '${HOME}/.local/jpod/containers'
   database:
      path: '${HOME}/.local/jpod/data/jpod.db'
      pool-size: 10
   images:
      path: '${HOME}/.local/jpod/images'
   networking:
      ip-v4-cidr: '172.23.0.1/16'
      ip-v6-cidr: 'fd12:3456:789a:1::/64'
      bridge: jc0
   domain-socket: '${HOME}/.local/jpod/unix.socket.jpod'


   ```
### NB. 
  _If you prefer, you can place the configuration as a separate copy and specify its path as a flag to the final compile daemon._
   ```sh
    sudo ./jcd -c ${PATH}..../settings.yml
   ```

<!-- USAGE EXAMPLES -->
## Compilation
_Inside the project folder you can create a `build` folder and run the following commands inside of it._

```sh
  cmake .. --preset <configure-preset>
  cmake --build
```
### NB:
  the configurated preset in this case 
  - unixlike-clang-debug
  - unixlike-clang-release
  - unixlike-gcc-debug   `(gcc-g++ present)`
  - unixlike-gcc-release `(gcc-g++ present)`
 
<!-- ROADMAP -->
## Roadmap

- [x] OCI container download (Docker Hub)
- [x] Jail Creation, Start Up, Shutdown
- [x] Domain Socket RPC and Stream communication
- [x] VNET and epair automatic creation
- [x] Jail networking `bridge + epair`
- [x] Jail Creation, Start Up, Shutdown
- [x] Container Shell forwarding
- [ ] PF integration and port forwarding
- [-] Instruction based Image creation with `(chroot)`
- [-] Publish containers to OCI Registry
- [ ] Separation of freebsd operations into shared libraries
- [ ] Swarm mode driver `similar to docker swarm`
- [ ] Kubernetes driver `facilitate integration with kubernetes`
- [ ] Nomad driver `facilitate support with Nomad`
- [ ] RCTL integration for resource limiting
- [ ] External Plugin API for external plugin development


<!-- LICENSE -->
## License

Distributed under the BSD-3 License. See `LICENSE.md` for more information.