from distutils.core import setup
import py2exe
 
setup(
    windows=[{"script":"pf.py"}],
    #options={"py2exe": {"includes":["sip"]}}
)
