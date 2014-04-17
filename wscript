## -*- Mode: python; py-indent-offset: 4; indent-tabs-mode: nil; coding: utf-8; -*-

import os
import Options
import os.path
import ns3waf
import sys
import types
# local modules


def options(opt):
    opt.tool_options('compiler_cc') 
    ns3waf.options(opt)

def configure(conf):
    ns3waf.check_modules(conf, ['core', 'network', 'internet'], mandatory = True)
    ns3waf.check_modules(conf, ['point-to-point', 'tap-bridge', 'csma'], mandatory = False)
    ns3waf.check_modules(conf, ['visualizer', 'mobility'], mandatory = False)
    conf.check_tool('compiler_cc')

    conf.env['ENABLE_PYTHON_BINDINGS'] = True
    conf.env['NS3_ENABLED_MODULES'] = []
    ns3waf.print_feature_summary(conf)



def build_dce_tests(module, bld):
    return

def build_dce_examples(module, bld):
    module.add_example(needed = ['core', 'network', 'dce', 'netlink', 'csma', 'point-to-point', 'dce-dnssec'], 
                       target='bin/dce-dns-simple',
                       source=['examples/dce-dns-simple.cc'])

    module.add_example(needed = ['core', 'network', 'dce', 'netlink', 'point-to-point', 'dce-dnssec', 'mobility', 'csma', 'visualizer'], 
                       linkflags=['-Wl,--dynamic-linker=' + os.path.abspath (bld.env['ELF_LOADER_PATH'] + '/ldso')],
                       target='bin/dce-dnssec',
                       source=['examples/dce-dnssec.cc'])

    module.add_example(needed = ['core', 'network', 'dce', 'netlink', 'point-to-point', 'dce-dnssec', 'mobility'], 
                       target='bin/dce-dns-reflection',
                       source=['examples/dce-dns-reflection.cc'])

def build(bld):    
    module_source = [
        'helper/bind9-helper.cc',
        'helper/unbound-helper.cc',
        'model/dns-packet.cc',
        ]
    module_headers = [
        'helper/bind9-helper.h',
        'helper/unbound-helper.h',
        'model/dns-packet.h',
        ]
    uselib = ns3waf.modules_uselib(bld, ['dce'])
    module = ns3waf.create_module(bld, name='dce-dnssec',
                                  source=module_source,
                                  headers=module_headers,
                                  use=uselib,
                                  includes=[],
                                  lib=['dl'])
    #build_dce_tests(module, bld)
    build_dce_examples(module, bld)
