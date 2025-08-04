#!/usr/bin/env python3
print("Content-Type: text/html\r")
print("\r")
# This will cause a 500 error
raise Exception("Intentional server error for testing")
