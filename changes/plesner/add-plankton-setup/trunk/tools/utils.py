# Reads a .list file into an array of strings
def read_lines_from(name):
  list = []
  for line in open(name):
    trimmed = line[:line.find('#')].strip()
    if len(trimmed) == 0: continue
    list.append(trimmed)
  return list
