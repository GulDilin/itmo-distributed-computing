# Distributed computing

### Setup dev env (for pre-commit checks)

1. Install `python 3.9` [\[download\]](https://www.python.org/downloads/release/python-390/)
1. Check installation with command `python --version`
1. Add `pre-commit` dependency
   You can use global python dependencies, but we recommend to use virtual env
   Variant 1: virtual env (recommended)

```shell
python -m venv venv
./venv/Scripts/activate     # for Windows
source venv/bin/activate     # for *NIX
python -m pip install pre-commit==3.4.0
```

Variant 2: global

```shell
python -m pip install pre-commit==3.4.0
```

1. Install `pre-commit` hook

```shell
pre-commit install -f
```
