def options(opt):
  opt.load('compiler_cxx waf_unit_test')

def configure(ctx):
  ctx.load('compiler_cxx waf_unit_test')

def build(bld):
  bld.program(features='test',
              source='vpacker64_test.cpp gtest/gtest-all.cc',
              includes = '.',
              target ='vpacker64_unitest',
              cxxflags = '-Wall -Wextra -Wformat=2  \
              -Wno-strict-aliasing -Wcast-qual \
              -Wcast-align -Wwrite-strings -Wfloat-equal \
              -Wpointer-arith',
              linkflags = '-pthread')

  bld.program(features='test',
              source='vpacker32_test.cpp gtest/gtest-all.cc',
              includes = '.',
              target ='vpacker32_unitest',
              cxxflags = '-Wall -Wextra -Wformat=2  \
              -Wno-strict-aliasing -Wcast-qual \
              -Wcast-align -Wwrite-strings -Wfloat-equal \
              -Wpointer-arith',
              linkflags = '-pthread')

  bld.shlib(source='libvpack.cpp',
            includes = '.',
            target='vpack',
            defines='NDEBUG',
            cxxflags = '-O2 -fomit-frame-pointer -march=nocona')

  from waflib.Tools import waf_unit_test
  bld.add_post_fun(waf_unit_test.summary)
