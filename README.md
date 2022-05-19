## 0. Introduction<br>

This repository contains one version of the source code for our NSDI'22 paper
["NetVRM: Virtual Register Memory for Programmable Networks"](https://www.usenix.org/conference/nsdi22/presentation/zhu).

## 1. Content<br>

- switch_code/<br>
  - p4/: p4 code for NetVRM in the Wide Area Network (WAN) scenario.<br>
    - includes/: packet header and parser.
    - `netvrm_wan.p4`: the main ingress and egress pipeline. 
    - `utils.p4`: the page table and counter record.
    - `shared_ht.p4`: the virtualized register memory of the first four stages.
    - `shared_ht_ext1.p4`: the virtualized register memory of the last four stages.
  - netvrm_c/<br>
    - src/
      - `application.hpp` describes the class `Application`, which represents an app instance. The class also contains a method to generate the application workload given the corresponding parameters.
      - `options.hpp` describes all the runtime arguments.
      - `switch.hpp` describes the class `Switch`.
    - tests/
      - `test.hpp` is the parent class `Test` and contains methods to commit updates to the data plane.
      - `equal-active-test.hpp` is the class `EqualActiveTest` inheriting from `Test`, and realizes the *Equal-Active* approach.
      - `equal-all-test.hpp` is the class `EqualAllTest` inheriting from `Test`, and realizes the *Equal-All* approach.
      - `netvrm-test.hpp` is the class `NetVRMTest` inheriting from `Test`, and realizes the *NetVRM* approach.
- `config.py`: some parameters to configure.
- `console.py`: the python script  to help run the experiments.
- `README.md`: This file.<br>

## 2. Environment requirement<br>

- Hardware<br>
  - Intel Tofino switch<br>
  - Servers with a DPDK-compatible NIC (we used an Intel XL710 for 40GbE QSFP+)<br>
- Software<br>
  - Intel P4 Studio (8.9.1)<br>
- Datasets<br>
  - [2019 passive CAIDA trace](https://data.caida.org/datasets/passive-2019/)

## 3. How to run<br>

- Configure the parameters in the files based on your environment<br>
  - `config.py`: provide the information of your servers (passwd, dir, etc.).<br>
- Environment setup<br>
  - Setup the switch<br>
    - Copy the files to the switch with `python console.py sync_switch`<br>
    - Setup the necessary environment variables to point to the appropriate locations.<br>
    - Compile `netvrm_wan.p4` in your P4 Studio.<br>
  - Setup the clients<br>
    - Please refer to [MoonGen](https://github.com/emmericp/MoonGen).<br>
- Compile and run the switch program<br>
  - `python console.py run_switch`<br>
- Other commands<br>
  - `python console.py replay_caida`: replay the CAIDA traces on the end-hosts.<br>
  - `python console.py kill_replay`: terminate the CAIDA trace replay.<br>
- Reproduce the results<br>
  - `python console.py run_hh`: run heavy hitter detection apps in WAN scenario.<br>
  - `python console.py run_opentcp`: run newly opened TCP connection detection apps in WAN scenario.<br>
  - `python console.py run_ss`: run superspreader detection apps in WAN scenario.<br>
  - `python console.py run_slo`: study the impact of the utility target.<br>
  - `python console.py run_ar`: study the impact of the number of application instances.<br>
  - `python console.py run_alloc_epoch`: study the impact of the allocation epoch.<br>

## 5. Contact<br>

For any question, please contact `hzhu at jhu dot edu`.
