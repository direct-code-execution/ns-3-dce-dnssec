## -*- Mode: python; py-indent-offset: 4; indent-tabs-mode: nil; coding: utf-8; -*-

import os
import Options
import os.path
import ns3waf
import sys
import types

def options(opt):
    opt.tool_options('compiler_cc') 
    ns3waf.options(opt)

def configure(conf):
    ns3waf.check_modules(conf, ['core', 'network', 'internet'], mandatory = True)
    ns3waf.check_modules(conf, ['point-to-point', 'tap-bridge', 'csma'], mandatory = False)
    ns3waf.check_modules(conf, ['dce'], mandatory = True)
    conf.check_tool('compiler_cc')
    ns3waf.print_feature_summary(conf)
    
def build_dce_tests(module, bld):
    return

def build_dce_examples(module, bld):
    module.add_example(needed = ['core', 'network', 'dce', 'csma', 'point-to-point'], 
                       target='bin/dce-dns-simple',
                       source=['examples/dce-dns-simple.cc'])

def build(bld):    
    module_source = [
    #    'helper/bind9-helper.cc',
        ]
    module_headers = [
    #    'helper/bind9-helper.h',
        ]
    uselib = ns3waf.modules_uselib(bld, ['dce'])
    module = ns3waf.create_module(bld, name='dce-dnssec',
                                  source=module_source,
                                  headers=module_headers,
                                  use=uselib,
                                  includes=[''],
                                  lib=['dl'])
    #build_dce_tests(module, bld)
    #bld.install_files('${PREFIX}/bin', 'build/bin/ns3test-dce', chmod=0755 )
    #bld.add_subdirs(['examples'])
    build_dce_examples(module, bld)
