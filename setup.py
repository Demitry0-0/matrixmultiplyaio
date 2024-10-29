from setuptools import Extension, setup

extensions = [Extension('matrixmultiplyaio',
                        sources=['matrixmultiplyaio.c'],
                        )
              ]

setup(name='matrixmultiplyaio', ext_modules=extensions)

