import torch
import shutil

VERSION = 'MCTS_1s_512_32_32'
MODEL_FILENAME = 'model_latest.pth'


def clamp(n, smallest, largest):
    return max(smallest, min(n, largest))


def double_to_int(value, scaling_factor, smallest, largest):
    x = round(value * scaling_factor)
    if not (smallest <= x <= largest):
        print(value, x)
    return str(clamp(x, smallest, largest))


def export2d(filename, parameters, scaling_factor, smallest, largest):
    with open(f'../parameters/{VERSION}/{filename}', 'w') as f:
        f.write('{')
        for i, line in enumerate(parameters):
            f.write('{')
            line = ','.join((double_to_int(v, scaling_factor, smallest, largest) for v in line))
            f.write(line + '}')
            if i+1 != len(parameters):
                f.write(',\n')
        f.write('}')


def export1d(filename, parameters, scaling_factor, smallest, largest):
    with open(f'../parameters/{VERSION}/{filename}', 'w') as f:
        line = ','.join((double_to_int(v, scaling_factor, smallest, largest) for v in parameters))
        f.write('{' + line + '}')


def main():
    model = torch.load(f'../{MODEL_FILENAME}')
    export2d('hidden1_layer_weights.txt', model.l1.weight.T.tolist(), 127, -32768, 32767)
    export1d('hidden1_layer_biases.txt', model.l1.bias.tolist(), 127, -32768, 32767)
    export2d('hidden2_layer_weights.txt', model.l2.weight.tolist(), 64, -128, 127)
    export1d('hidden2_layer_biases.txt', model.l2.bias.tolist(), 127*64, -32768, 32767)
    export2d('hidden3_layer_weights.txt', model.l3.weight.tolist(), 64, -128, 127)
    export1d('hidden3_layer_biases.txt', model.l3.bias.tolist(), 127*64, -32768, 32767)
    export1d('hidden4_layer_weights.txt', model.l4.weight.tolist()[0], 64, -128, 127)
    with open(f'../parameters/{VERSION}/hidden4_layer_bias.txt', 'w') as f:
        f.write(str(model.l4.bias.item()))
    shutil.copyfile(f'../{MODEL_FILENAME}', f'../parameters/{VERSION}/{MODEL_FILENAME}')


if __name__ == '__main__':
    main()
