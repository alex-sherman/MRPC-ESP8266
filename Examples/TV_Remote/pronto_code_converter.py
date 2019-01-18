# Some sites give IR codes in this weird pronto format
# Cut off the preamble and footer and find which value represents a 1 in the spacing

code = "0000 0016 0000 0016 0000 0016 0000 003F 0000 003F 0000 003F 0000 0016 0000 003F 0000 0016 0000 0016 0000 0016 0000 0016 0000 0016 0000 0016 0000 0016 0000 0016 0000 0016 0000 003F 0000 0016 0000 003F 0000 003F 0000 0016 0000 0016 0000 003F 0000 003F 0000 0016 0000 003F 0000 0016 0000 0016 0000 003F 0000 003F 0000 0016 0000"

print(int(''.join(list(map(lambda x: "1" if x == '003F' else "0", code.split(' ')[1::2]))), 2))