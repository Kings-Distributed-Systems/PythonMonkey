import os
import pmpm

WORK_DIR = os.path.join(
  os.path.realpath(os.path.dirname(__file__)),
  "pythonmonkey"
)

def main():
  pmpm.main(WORK_DIR) # cd pythonmonkey && npm i

if __name__ == "__main__":
  main()
