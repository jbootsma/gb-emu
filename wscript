#   Copyright (C) 2017  James Bootsma <jrbootsma@gmail.com>
#
#   This program is free software: you can redistribute it and/or modify
#   it under the terms of the GNU General Public License as published by
#   the Free Software Foundation, either version 3 of the License, or
#   (at your option) any later version.
#
#   This program is distributed in the hope that it will be useful,
#   but WITHOUT ANY WARRANTY; without even the implied warranty of
#   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#   GNU General Public License for more details.
#
#   You should have received a copy of the GNU General Public License
#   along with this program.  If not, see <http://www.gnu.org/licenses/>.

from waflib.extras import msvs
from waflib import Build
from waflib.TaskGen import feature, before

### Common Flags setup.

# For each tool that should have a set of common flags there should be a set
# of nested dictionaries such that looking up <tool>_flags[<platform_tool>][<variant>]
# gives the set of flags to append. A None key can be used for things that should
# be common across all variants.

cxx_flags_msvc = {
    None :      ['/volatile:iso', '/Zi', '/FS', '/nologo', '/W4', '/WX',
                 '/utf-8'],
    'release' : ['/O2', '/EHsc', '/GL', '/Gw', '/Gy', '/fp:fast'],
    'debug' :   ['/Od', '/EHscr', '/GF', '/RTC1', '/Za'],
}

cxx_flags = {
    'msvc' : cxx_flags_msvc,
}

link_flags_msvc = {
    None :      ['/nologo', '/wx', '/debug', '/incremental:no'],
    'release' : ['/ltcg', '/opt:ref,icf'],
    'debug' :   [],
}

link_flags = {
    'msvc' : link_flags_msvc,
}

@feature('common_flags')
@before('process_source', 'apply_link')
def apply_common_flags(self):
    def add_flags(target_var, tool_var, flags_dict):
        for tool in None, self.env[tool_var]:
            for variant in None, self.bld.variant:
                try:
                    self.env.append_unique(target_var, flags_dict[tool][variant])
                except KeyError:
                    pass
    if 'cxx' in self.features:
        add_flags('CXXFLAGS', 'COMPILER_CXX', cxx_flags)
    if 'cxxprogram' in self.features:
        add_flags('LINKFLAGS', 'COMPILER_CXX', link_flags)
    if 'cprogram' in self.features:
        add_flags('LINKFLAGS', 'COMPILER_C', link_flags)

### Standard commands

def options(ctx):
    ctx.load('msvs')
    ctx.load('compiler_cxx')

def configure(ctx):
    ctx.load('msvs')
    ctx.load('compiler_cxx')

def build(ctx):
    defines = []
    
    if ctx.variant == 'release':
        defines += ['NDEBUG']

    ctx.program(
        source = ctx.path.ant_glob('src/**/*.cpp'),
        target = 'gb-emu',
        features = 'common_flags',
        includes = ['src'],
        defines = defines,
    )

class ReleaseBuild(Build.BuildContext):
    cmd = 'build'
    variant = 'release'

class DebugBuild(Build.BuildContext):
    '''executes the build with debug variant'''
    cmd = 'dbuild'
    fun = 'build'
    variant = 'debug'

### MSVS customization

def wrap_node_class(cls):
    class Wrapper(cls):
        def get_build_command(self, props):
            return super(Wrapper, self).get_build_command(props).replace('build', 'dbuild')
        def get_rebuild_command(self, props):
            return super(Wrapper, self).get_rebuild_command(props).replace('build', 'dbuild')
    return Wrapper

node_wrap_dir = {
    'vsnode_target' : msvs.vsnode_target,
    'vsnode_build_all' : msvs.vsnode_build_all,
    'vsnode_install_all' : msvs.vsnode_install_all,
}

class MsvsGenerator(msvs.msvs_generator):
    variant = 'debug'
    numver = '12.00'
    vsver = '14'
    platform_toolset_ver = 'v140'
    default_project = 'gb-emu'

    def init(self):
        self.projects_dir = self.srcnode.make_node('.vs/projects')
        self.projects_dir.mkdir()
        self.solution_name = '.vs/gb-emu.sln'
        for (prop, cls) in node_wrap_dir.items():
            setattr(self, prop, wrap_node_class(cls))
        super(MsvsGenerator, self).init()
