#
# Vocabulary generator using Markov Chains.
# (c) bodrich, 2018
# Original: https://repl.it/repls/FatalWornWifi
#
import random, sys, re

PLACES = open("ahenn.txt", "r").read().split("\n")


class Mdict:
    def __init__(self):
        self.d = {}

    def __getitem__(self, key):
        if key in self.d:
            return self.d[key]
        else:
            raise KeyError(key)

    def add_key(self, prefix, suffix):
        if prefix in self.d:
            self.d[prefix].append(suffix)
        else:
            self.d[prefix] = [suffix]

    def get_suffix(self, prefix):
        l = self[prefix]
        return random.choice(l)


class MName:
    def __init__(self, chainlen=3):
        if chainlen > 10 or chainlen < 1:
            print("Chain length must be between 1 and 10, inclusive")
            sys.exit(0)

        self.mcd = Mdict()
        oldnames = []
        self.chainlen = chainlen

        for l in PLACES:
            l = l.strip()
            oldnames.append(l)
            s = " " * chainlen + l
            for n in range(0, len(l)):
                self.mcd.add_key(s[n:n + chainlen], s[n + chainlen])
            self.mcd.add_key(s[len(l):len(l) + chainlen], "\n")

    def normalize(self, name):
        name = name.replace("`", "").replace("'", "");

        if (len(name) >= 1):
            if (name[-1] == '-' or name[-1] == 'ъ'):
                return '';
        if (len(name) >= 2):
            if (name[-2] == '-'):
                return ''
        if (name.count('-') > 1):
            return '';

        return name

    def New(self):
        prefix = " " * self.chainlen
        name = ""
        suffix = ""
        while True:
            suffix = self.mcd.get_suffix(prefix)
            if suffix == "\n" or len(name) > 10:
                break
            else:
                name = name + suffix
                prefix = prefix[1:] + suffix

        return self.normalize(name)

NAMES = [] + PLACES

for i in range(5000):
    name = MName(3).New()
    if name not in NAMES and len(name) > 6:
        print(name)
        NAMES.append(name)
