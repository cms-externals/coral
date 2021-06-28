import sys
if sys.platform == 'linux2':
  import DLFCN
  sys.setdlopenflags( DLFCN.RTLD_NOW | DLFCN.RTLD_GLOBAL )
  from liblcg_PyCoral import *
  import liblcg_PyCoral
  __doc__=liblcg_PyCoral.__doc__
  del liblcg_PyCoral
  del DLFCN
elif sys.platform == 'linux':# CORALCOOL-2977 uses python 3.6 and up DLFCN replaced by os
  import os
  sys.setdlopenflags( os.RTLD_NOW | os.RTLD_GLOBAL )
  from liblcg_PyCoral import *
  import liblcg_PyCoral
  __doc__=liblcg_PyCoral.__doc__
  del liblcg_PyCoral
  del os
else:
  if sys.platform == 'darwin':
    from liblcg_PyCoral import *
    import liblcg_PyCoral
    __doc__=liblcg_PyCoral.__doc__
    del liblcg_PyCoral
  else:
    from lcg_PyCoral import *
    import lcg_PyCoral
    __doc__=lcg_PyCoral.__doc__
    del lcg_PyCoral
del sys
