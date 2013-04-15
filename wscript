## -*- Mode: python; py-indent-offset: 4; indent-tabs-mode: nil; coding: utf-8; -*-

import os
import Options
import os.path
import ns3waf
import sys
import types
# local modules
import wutils

def options(opt):
    opt.tool_options('compiler_cc') 
    ns3waf.options(opt)
    opt.add_option('--enable-opt',
                   help=('Enable use of DCE and NS-3 optimized compilation'),
                   dest='enable_opt', action='store_true',
                   default=False)    
    opt.add_option('--cwd',
                   help=('Set the working directory for a program.'),
                   action="store", type="string", default=None,
                   dest='cwd_launch')
    opt.add_option('--command-template',
                   help=('Template of the command used to run the program given by --run;'
                         ' It should be a shell command string containing %s inside,'
                         ' which will be replaced by the actual program.'),
                   type="string", default=None, dest='command_template')
    opt.add_option('--run',
                   help=('Run a locally built program; argument can be a program name,'
                         ' or a command starting with the program name.'),
                   type="string", default='', dest='run')
    opt.add_option('--visualize',
                   help=('Modify --run arguments to enable the visualizer'),
                   action="store_true", default=False, dest='visualize')
    opt.add_option('--valgrind',
                   help=('Change the default command template to run programs and unit tests with valgrind'),
                   action="store_true", default=False,
                   dest='valgrind')

def configure(conf):
    os.environ['PKG_CONFIG_PATH'] = os.path.join(conf.env.PREFIX, 'lib', 'pkgconfig')
    ns3waf.check_modules(conf, ['core', 'network', 'internet'], mandatory = True)
    ns3waf.check_modules(conf, ['point-to-point', 'tap-bridge', 'csma'], mandatory = False)
    ns3waf.check_modules(conf, ['dce', 'netlink'], mandatory = True)
    ns3waf.check_modules(conf, ['visualizer', 'mobility'], mandatory = False)
    conf.check_tool('compiler_cc')
    conf.env.append_value('LINKFLAGS', '-Wl,--dynamic-linker=' +
                          os.path.abspath (conf.env.PREFIX + '/lib/ldso'))
    conf.env['ENABLE_PYTHON_BINDINGS'] = True
    conf.env['EXAMPLE_DIRECTORIES'] = '.'
    conf.env['NS3_ENABLED_MODULES'] = []
    ns3waf.print_feature_summary(conf)
    
def build_dce_tests(module, bld):
    return

def build_dce_examples(module, bld):
    module.add_example(needed = ['core', 'network', 'dce', 'netlink', 'csma', 'point-to-point', 'dce-dnssec'], 
                       target='bin/dce-dns-simple',
                       source=['examples/dce-dns-simple.cc'])

    module.add_example(needed = ['core', 'network', 'dce', 'netlink', 'point-to-point', 'dce-dnssec', 'mobility', 'csma', 'visualizer'], 
                       target='bin/dce-dnssec',
                       source=['examples/dce-dnssec.cc'])

def _get_all_task_gen(self):
    for group in self.groups:
        for taskgen in group:
            yield taskgen

def build(bld):    
    bld.env['NS3_MODULES_WITH_TEST_LIBRARIES'] = []
    bld.env['NS3_ENABLED_MODULE_TEST_LIBRARIES'] = []
    bld.env['NS3_SCRIPT_DEPENDENCIES'] = []
    bld.env['NS3_RUNNABLE_PROGRAMS'] = []
    bld.env['NS3_RUNNABLE_SCRIPTS'] = []
    module_source = [
        'helper/bind9-helper.cc',
        'helper/unbound-helper.cc',
        ]
    module_headers = [
        'helper/bind9-helper.h',
        'helper/unbound-helper.h',
        ]
    uselib = ns3waf.modules_uselib(bld, ['dce'])
    module = ns3waf.create_module(bld, name='dce-dnssec',
                                  source=module_source,
                                  headers=module_headers,
                                  use=uselib,
                                  includes=['/usr/lib/ruby/1.8/x86_64-linux/'],
                                  lib=['dl', 'ruby1.8'])
    #build_dce_tests(module, bld)
    #bld.install_files('${PREFIX}/bin', 'build/bin/ns3test-dce', chmod=0755 )
    #bld.add_subdirs(['examples'])
    build_dce_examples(module, bld)
    # Write the build status file.
    build_status_file = os.path.join(bld.out_dir, 'build-status.py')
    out = open(build_status_file, 'w')
    out.write('#! /usr/bin/env python\n')
    out.write('\n')
    out.write('# Programs that are runnable.\n')
    out.write('ns3_runnable_programs = ' + str(bld.env['NS3_RUNNABLE_PROGRAMS']) + '\n')
    out.write('\n')
    out.write('# Scripts that are runnable.\n')
    out.write('ns3_runnable_scripts = ' + str(bld.env['NS3_RUNNABLE_SCRIPTS']) + '\n')
    out.write('\n')
    out.close()

    wutils.bld = bld
    bld.__class__.all_task_gen = property(_get_all_task_gen)
    Options.cwd_launch = bld.path.abspath()
    if Options.options.run:
        # Check that the requested program name is valid
        program_name, dummy_program_argv = wutils.get_run_program(Options.options.run, wutils.get_command_template(bld.env))

        # When --run'ing a program, tell WAF to only build that program,
        # nothing more; this greatly speeds up compilation when all you
        # want to do is run a test program.
        Options.options.targets += ',' + os.path.basename(program_name)
        if getattr(Options.options, "visualize", False):
            program_obj = wutils.find_program(program_name, bld.env)
            program_obj.use.append('NS3_VISUALIZER')
        for gen in bld.all_task_gen:
            if type(gen).__name__ in ['task_gen', 'ns3header_taskgen', 'ns3moduleheader_taskgen']:
                gen.post()
        bld.env['PRINT_BUILT_MODULES_AT_END'] = False 

from waflib import Context, Build
class Ns3ShellContext(Context.Context):
    """run a shell with an environment suitably modified to run locally built programs"""
    cmd = 'shell'

    def execute(self):

        # first we execute the build
	bld = Context.create_context("build")
	bld.options = Options.options # provided for convenience
	bld.cmd = "build"
	bld.execute()

        # Set this so that the lists won't be printed when the user
        # exits the shell.
        bld.env['PRINT_BUILT_MODULES_AT_END'] = False

        if sys.platform == 'win32':
            shell = os.environ.get("COMSPEC", "cmd.exe")
        else:
            shell = os.environ.get("SHELL", "/bin/sh")

        env = bld.env
        os_env = {
            'NS3_MODULE_PATH': os.pathsep.join(env['NS3_MODULE_PATH']),
            'NS3_EXECUTABLE_PATH': os.pathsep.join(env['NS3_EXECUTABLE_PATH']),
            }
        wutils.run_argv([shell], env, os_env)

def shutdown(ctx):
    bld = wutils.bld
    if wutils.bld is None:
        return
    env = bld.env

    if Options.options.run:
        wutils.run_program(Options.options.run, env, wutils.get_command_template(env),
                           visualize=Options.options.visualize)
        raise SystemExit(0)

