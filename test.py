

with open("/dev/input/mouse0", 'rb') as inputFile:
    while True:
        k = inputFile.read(3)
        # print(list(map(ord, k)))
        print(k)