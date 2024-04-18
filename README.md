# JACC (Jokkes Amazing Compiler Compiler)

This project aims to process an input grammar and output a parser for said grammar.

Currently it is using Flex+Bison to parse the input grammar.

## Progress list

- [X] Parse grammar files
- [X] Generate FIRST sets
- [ ] Generate FOLLOW sets
- [ ] Generate LR item collection
- [ ] Generate Parser
- [ ] Implement semantic actions

## Caveats and gotchas

These are things that were simply natural to me. However, I later discovered while researching that others might not agree so it felt worth while to note them down.

### We assume the starting symbol is the top one in the grammar.
For example
```
S : A
  | B;
A : 'a';
B : 'b';
```
is perfectly fine, but controversely
```
A : 'a';
B : 'b';
S : A
  | B;
```
is not okay, even though they describe the exact same thing.


### I prefer choices over multiple rules
Some tools and articles I've seen tend to use multiple rules instead of choices.

Using the above grammar as an example:
```
S : A
  | B;
A : 'a';
B : 'b';
```
Notice how the rule for `S` has two choices, either `A` or `B`.  


What I've been seeing, but don't really like, is the equivalent grammar using notation like:
```
S : A;
S : B;
A : 'a';
B : 'b';
```

At the time of writing, the latter doesn't work in JACC. It won't crash, but it'll likely give you some wrong results.
for example, during first set generation, after it has generated the first set of `S : A;` and is about to generate the first set of
`S : B;`, it sees that S already has a first set generated and returns that first set. This effectively ignores the second rule.
