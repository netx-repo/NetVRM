import sys, os, time, subprocess, random, pdb
import paramiko, scipy
from scipy import stats
from config import *

class Console(object):
    def __init__(self, server_names, switch_name):
        self.trace_server_names = server_names
        self.switch_name = switch_name

        self.local_p4_dir = "switch_code"
        self.remote_switch_dir = switch_netvrm_dir

        self.servers = []
        for server_name in self.trace_server_names:
            client = paramiko.SSHClient()
            client.load_system_host_keys()
            try:
                client.connect(hostname = server_name, username = "user", password = host_pw,allow_agent=False,look_for_keys=False)
            except:
                print "connecting to", server_name, "failed"
                raise
            self.servers.append((server_name, client))

        switch = paramiko.SSHClient()
        switch.load_system_host_keys()
        switch.connect(hostname = self.switch_name, username = "root", password = switch_pw,allow_agent=False,look_for_keys=False)
        self.switch = (self.switch_name, switch)


        # default params
        self.type = "hh"
        self.slo = 0.02
        self.total_hw = 16
        self.prog_name = "netvrm_wan"
        self.ar = 4
        self.print_flag = False
        self.alloc_epoch = 1

        self.min_idx = 0

        print "========init completed========"

    """
    the function that run locally
    """

    def sync_switch(self):
        cmd = "sshpass -p %s rsync -r %s root@%s:%s" % (switch_pw, self.local_p4_dir, self.switch_name, self.remote_switch_dir)
        subprocess.call(cmd, shell = True)

    """
    functional methods
    """

    def exe(self, client, cmd, with_print=False):
        (client_name, client_shell) = client
        stdin, stdout, stderr = client_shell.exec_command(cmd)
        if with_print:
            print client_name, ":", stdout.read(), stderr.read()

    def exe_ptf(self, client, cmd, with_print=False):
        (client_name, client_shell) = client
        stdin, stdout, stderr = client_shell.exec_command(cmd, get_pty=True)

        sat_list = []
        end_flag = 0
        start_flag = 0
        drop_cnt, reject_cnt = 0, 0
        for line in iter(stdout.readline, ""):
            line = line.encode('utf-8')
            if with_print:
                print "line:", line[:-2]
                sys.stdout.flush()
            if "Error" in line or "error" in line or "ERROR" in line:
                print "Error in line, exit"
                return None, None, None
            if "Segmentation fault" in line:
                print "Segmentation fault in line, exit"
                return None, None, None
            if str_trace_end in line:
                # current trace_end
                self.min_idx += 1
                self.kill_replay()
                time.sleep(8)
                self.replay_caida()
            if end_flag:
                a = str(line)[:-2]
                if "cnt_pair" in a:
                    eles = a.split(";")
                    sat_cnt = int(eles[-2])
                    total_cnt = int(eles[-1])
                    sat_list.append((sat_cnt, total_cnt))

                elif "(dropped, rejected)" in a:
                    eles = a.split(";")
                    drop_cnt = int(eles[-2])
                    reject_cnt = int(eles[-1])

            if str_exit in line:
                break

            if (str_main_start in line) and (not start_flag):
                start_flag = 1
                print "exp starts","="*10
                print "start trace replay on server..."
                self.replay_caida()

            if (str_main_end in line) and (not end_flag):
                print "exp ends","="*10
                end_flag = 1

        return sat_list, drop_cnt, reject_cnt

    def sudo_exe(self, client, cmd, with_print=False):
        cmdheader = "echo '%s' | sudo -S " %(host_pw)
        cmd = cmdheader + cmd
        (client_name, client_shell) = client
        print "cmd in sudo exe", cmd
        stdin, stdout, stderr = client_shell.exec_command(cmd)
        if with_print:
            print client_name, ":", stdout.read(), stderr.read()
            stdout.flush()
            stderr.flush()

    """
    related to switch
    """
    def kill_p4_c(self):
        self.exe(self.switch, "ps -ef | grep switchd | grep -v grep | " \
            "awk '{print $2}' | xargs kill -9")

        self.exe(self.switch, "ps -ef | grep run_p4_test | grep -v grep | " \
            "awk '{print $2}' | xargs kill -9")

        self.exe(self.switch, "ps -ef | grep CP | grep -v grep | " \
            "awk '{print $2}' | xargs kill -9")

    def run_switch(self):
        cmd = "cd %s;. ./set_sde.bash; cd %s;./run.sh -scheme %d -app_num %d -total_hw %d -scene_type %d -slo %f -alloc_epoch %d" \
            % (switch_sde_dir, netvrm_wan_dir, self.static_flag, self.ar*ar_factor, self.total_hw, scene_type_list.index(self.type), self.slo, self.alloc_epoch)

        print cmd
        return self.exe_ptf(self.switch, cmd, self.print_flag)

    """
    related to replay trace
    """
    def kill_replay(self):
        cmd = "pkill MoonGen"
        for client in self.servers:
            if client[0] in self.trace_server_names:
                self.sudo_exe(client, cmd)
        time.sleep(1)

    def replay_caida(self):
        trace_name = "equinix-nyc.dirA.20190117-13" + str(self.min_idx%30).zfill(2) + "00.UTC.anon.pcap"

        cmd = "cd /home/user/MoonGen; echo 'user' | sudo -S build/MoonGen examples/pcap/replay-pcap.lua -r 1 -s 1 -l 0 /home/user/netvrm/traces/caida/" \
            + trace_name + " 2>&1 &"

        for client in self.servers:
            if client[0] in self.trace_server_names:
                print client[0],":",cmd
                self.exe(client, cmd, with_print=False)

    """
    related to draw figures
    """
    def return_mean_and_5(self, input_list):
        task_num = len(input_list)
        filter_list = [i for i in input_list if i >= 0]
        print len(filter_list), "out of", task_num, "has traffic"
        input_mean = round(scipy.mean(filter_list),5)
        input_median = stats.scoreatpercentile(filter_list, 50)
        input_1 = stats.scoreatpercentile(filter_list, 1)
        input_5 = stats.scoreatpercentile(filter_list, 5)
        input_10 = stats.scoreatpercentile(filter_list, 10)
        return [input_mean, round(input_5, 5), round(input_10, 5)]

    def calc_sat_from_cnt(self, cnt_tuple_list):
        sat_list = []
        for idx, (sat_cnt, total_cnt) in enumerate(cnt_tuple_list):
            try:
                sat_list.append(round(float(sat_cnt)/total_cnt, 5))
                print idx, sat_cnt, total_cnt
            except:
                print "No traffic for task", idx
                sat_list.append(-1)
        return sat_list

    def reset_default_config(self):
        self.type = "hh"
        self.ar = 4
        self.slo = 0.02
        self.alloc_epoch = 1
        self.total_hw = 14

    def run_wan(self, scene_type="hh", debug_flag=False):

        total_hw_list = [16,17]
        static_flag = [3]

        self.reset_default_config()

        self.type = scene_type
        self.prog_name = "netvrm_wan"
        self.print_flag = True

        static_list = []
        equal_active_list = []
        netvrm_list = []
        drop_rate_list = []
        rej_rate_list = []

        for flag in static_flag:
            self.static_flag = flag
            for hw in total_hw_list:
                self.total_hw = hw - 2
                self.min_idx = 0
                start_time = time.time()
                self.kill_replay()
                print "Kill existing p4..."
                self.kill_p4_c()
                time.sleep(8)

                sat_cnt_list, drop_cnt, reject_cnt = self.run_switch()
                if sat_cnt_list!= None and drop_cnt!=None and reject_cnt!=None:
                    break
                else:
                    print "scene_type", self.type, "static_flag", self.static_flag, "total_hw", self.total_hw
                    sys.exit(1)

                sat_list = self.calc_sat_from_cnt(sat_cnt_list)
                mean, tail5,_ = self.return_mean_and_5(sat_list)
                print "mean, tail5", (mean, tail5)
                print "vertical_line:scene_type", self.type, "static_flag", self.static_flag, "total_hw", self.total_hw
                print "(drop_cnt, reject_cnt)", (drop_cnt, reject_cnt)
                print "elapsed time", time.time()-start_time
                print "sat_list", sat_list
                sys.stdout.flush()
                if len(sat_cnt_list)+drop_cnt+reject_cnt != self.ar*ar_factor:
                    print "Error: need to rerun"
                    continue
                print "="*30
                if flag == 0:
                    static_list.append([mean, tail5])
                elif flag == 1:
                    equal_active_list.append([mean, tail5])
                elif flag == 2:
                    netvrm_list.append([mean, tail5])
                    drop_rate = round(float(drop_cnt)/(self.ar*ar_factor), 5)
                    drop_rate_list.append(drop_rate)
                    print drop_cnt, "drop tasks", drop_rate
                    rej_rate = round(float(reject_cnt)/(self.ar*ar_factor), 5)
                    rej_rate_list.append(rej_rate)
                    print reject_cnt, "reject tasks", rej_rate

        print "="*15, "run wan", self.type, "final results", "="*15
        print "drop_rate", drop_rate_list
        print "reject_rate", rej_rate_list
        print "static_list", static_list
        print "equal_active_list", equal_active_list
        print "netvrm_list", netvrm_list

    def run_slo(self):

        slo_list = [0.04, 0.03, 0.01]
        static_flag = [0,1,2]

        self.reset_default_config()

        self.type = "hh"
        self.prog_name = "netvrm_wan"
        self.print_flag = True

        static_list = []
        equal_active_list = []
        netvrm_list = []
        drop_rate_list = []
        rej_rate_list = []

        for flag in static_flag:
            self.static_flag = flag
            for slo in slo_list:
                self.slo = slo
                self.min_idx = 0
                start_time = time.time()
                self.kill_replay()
                print "Kill existing p4..."
                self.kill_p4_c()
                time.sleep(8)

                sat_cnt_list, drop_cnt, reject_cnt = self.run_switch()
                if sat_cnt_list!= None and drop_cnt!=None and reject_cnt!=None:
                    break
                else:
                    print "scene_type", self.type, "static_flag", self.static_flag, "slo", self.slo
                    sys.exit(1)

                print "sat_cnt_list", sat_cnt_list
                sat_list = self.calc_sat_from_cnt(sat_cnt_list)
                mean, tail5,_ = self.return_mean_and_5(sat_list)
                print "mean, tail5", (mean, tail5)
                print "vertical_line:scene_type", self.type, "static_flag", self.static_flag, "total_hw", self.total_hw, "slo", self.slo
                print "(drop_cnt, reject_cnt)", (drop_cnt, reject_cnt)

                print "elapsed time", time.time()-start_time
                print "sat_list", sat_list
                sys.stdout.flush()
                if len(sat_cnt_list)+drop_cnt+reject_cnt != self.ar*ar_factor:
                    print "Error: need to rerun"
                    continue
                print "="*30
                if flag == 0:
                    static_list.append([mean, tail5])
                elif flag == 1:
                    equal_active_list.append([mean, tail5])
                elif flag == 2:
                    netvrm_list.append([mean, tail5])
                    drop_rate_list.append(round(float(drop_cnt)/(self.ar*ar_factor), 5))
                    print drop_cnt, "drop tasks"
                    rej_rate_list.append(round(float(reject_cnt)/(self.ar*ar_factor), 5))
                    print reject_cnt, "reject tasks"
        print "="*15, "run slo", self.type, "final results", "="*15
        print "drop_rate", drop_rate_list
        print "reject_rate", rej_rate_list
        print "static_list", static_list
        print "equal_active_list", equal_active_list
        print "netvrm_list", netvrm_list
    
    def run_ar(self):
        ar_list = [2,3,5]
        static_flag = [0,1,2]

        self.reset_default_config()
        self.type = "hh"
        self.prog_name = "netvrm_wan"
        self.print_flag = True

        static_list = []
        equal_active_list = []
        netvrm_list = []
        drop_rate_list = []
        rej_rate_list = []

        for flag in static_flag:
            self.static_flag = flag
            for ar in ar_list:
                self.ar = ar
                self.min_idx = 0

                start_time = time.time()
                self.kill_replay()
                print "Kill existing p4..."
                self.kill_p4_c()
                time.sleep(8)

                sat_cnt_list, drop_cnt, reject_cnt = self.run_switch()
                if sat_cnt_list!= None and drop_cnt!=None and reject_cnt!=None:
                    break
                else:
                    print "scene_type", self.type, "static_flag", self.static_flag, "ar", self.ar
                    sys.exit(1)

                print "sat_cnt_list", sat_cnt_list
                sat_list = self.calc_sat_from_cnt(sat_cnt_list)
                mean, tail5,_ = self.return_mean_and_5(sat_list)
                print "mean, tail5", (mean, tail5)
                print "vertical_line:scene_type", self.type, "static_flag", self.static_flag, "total_hw", self.total_hw, "ar", self.ar
                print "(drop_cnt, reject_cnt)", (drop_cnt, reject_cnt)

                print "elapsed time", time.time()-start_time
                print "sat_list", sat_list
                sys.stdout.flush()
                if len(sat_cnt_list)+drop_cnt+reject_cnt != self.ar*ar_factor:
                    print "Error: need to rerun"
                    continue
                print "="*30
                if flag == 0:
                    static_list.append([mean, tail5])
                elif flag == 1:
                    equal_active_list.append([mean, tail5])
                elif flag == 2:
                    netvrm_list.append([mean, tail5])
                    drop_rate_list.append(round(float(drop_cnt)/(self.ar*ar_factor), 5))
                    print drop_cnt, "drop tasks"
                    rej_rate_list.append(round(float(reject_cnt)/(self.ar*ar_factor), 5))
                    print reject_cnt, "reject tasks"
        print "="*15, "run ar", self.type, "final results", "="*15
        print "drop_rate", drop_rate_list
        print "reject_rate", rej_rate_list
        print "static_list", static_list
        print "equal_active_list", equal_active_list
        print "netvrm_list", netvrm_list

    def run_alloc_epoch(self):

        alloc_epoch_list = [2]
        static_flag = [2]
        self.reset_default_config()

        self.type = "hh"
        self.prog_name = "netvrm_wan"
        self.print_flag = True

        static_list = []
        equal_active_list = []
        netvrm_list = []
        drop_rate_list = []
        rej_rate_list = []

        for flag in static_flag:
            self.static_flag = flag
            for alloc_epoch in alloc_epoch_list:
                self.alloc_epoch = alloc_epoch
                self.min_idx = 0

                start_time = time.time()
                self.kill_replay()
                print "Kill existing p4..."
                self.kill_p4_c()
                time.sleep(8)

                sat_cnt_list, drop_cnt, reject_cnt = self.run_switch()
                if sat_cnt_list!= None and drop_cnt!=None and reject_cnt!=None:
                    break
                else:
                    print "scene_type", self.type, "static_flag", self.static_flag, "alloc_epoch", self.alloc_epoch
                    sys.exit(1)

                sat_list = self.calc_sat_from_cnt(sat_cnt_list)
                mean, tail5,_ = self.return_mean_and_5(sat_list)
                print "mean, tail5", (mean, tail5)
                print "vertical_line:scene_type", self.type, "static_flag", self.static_flag, "total_hw", self.total_hw, "alloc_epoch",self.alloc_epoch
                print "(drop_cnt, reject_cnt)", (drop_cnt, reject_cnt)
                print "elapsed time", time.time()-start_time
                print "sat_list", sat_list
                sys.stdout.flush()
                if len(sat_cnt_list)+drop_cnt+reject_cnt != self.ar*ar_factor:
                    print "Error: need to rerun"
                    continue
                print "="*30
                if flag == 1:
                    static_list.append([mean, tail5])
                elif flag == 0:
                    equal_active_list.append([mean, tail5])
                elif flag == 2:
                    netvrm_list.append([mean, tail5])
                    drop_rate_list.append(round(float(drop_cnt)/(self.ar*ar_factor), 5))
                    print drop_cnt, "drop tasks"
                    rej_rate_list.append(round(float(reject_cnt)/(self.ar*ar_factor), 5))
                    print reject_cnt, "reject tasks"
        print "="*15, "run alloc_epoch", self.type, "final results", "="*15
        print "drop_rate", drop_rate_list
        print "reject_rate", rej_rate_list
        print "static_list", static_list
        print "equal_active_list", equal_active_list
        print "netvrm_list", netvrm_list

if __name__ == "__main__":
    if len(sys.argv) <= 1:
        print_usage()
        sys.exit(0)

    console = Console(server_name_list, switch_name)

    if sys.argv[1] == "sync_switch":
        console.sync_switch()
    elif sys.argv[1] == "run_switch":
        console.run_switch()
    elif sys.argv[1] == "run_hh":
        console.run_wan("hh")
    elif sys.argv[1] == "run_opentcp":
        console.run_wan("opentcp")
    elif sys.argv[1] == "run_ss":
        console.run_wan("ss")
    elif sys.argv[1] == "run_mix":
        console.run_wan("mix")
    elif sys.argv[1] == "run_slo":
        console.run_slo()
    elif sys.argv[1] == "run_ar":
        console.run_ar()
    elif sys.argv[1] == "run_alloc_epoch":
        console.run_alloc_epoch()
    elif sys.argv[1] == "replay_caida":
        console.replay_caida()
    elif sys.argv[1] == "kill_replay":
        console.kill_replay()
    else:
        print "not supported..."
