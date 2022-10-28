

def removeLines(file):
    with open(file, 'r') as f:
        dados = f.readlines()
    for linha in range(len(dados)):
        dados[linha] = dados[linha][3:]
    with open('codigo.txt', 'w') as f:
        for texto in dados:
            f.write(texto)
#removeLines('codigo.txt')
