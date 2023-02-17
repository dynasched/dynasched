from setuptools import setup

setup(
    name="dsched-fifo",
    install_requires="dsched",
    entry_points={"dsched": ["fifo = fifo"]},
    py_modules=["fifo"],
)

