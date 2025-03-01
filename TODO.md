# :rocket: Liquidity Beggars :rocket:

## :memo: Project timeline:

- ✅ 26/02 - 1st meeting - idea discussion and task division
- ✅ 28/02 - 2nd meeting - all classes done, code merge
- ✅ 01/03 - 3rd meeting - main run and first code version working
- [ ] 04/03 - 3rd meeting - analysis done + polishing code + refactor
- [ ] 08/03 - 4th meeting - report done

## :pushpin: Backlog:
- ✅ Create project structure (classes, mathods, main)
- ✅ Implement logic of `OrderBook` class
- ✅ Implement logic of `Trader` class 
- ✅ Implement logic of `Exchange` class  
- ✅ Implement logic of `Order` class
- ✅ Write unit tests for `OrderBook` class
- ✅ Write unit tests for `Trader` class
- ✅ Write unit tests for `Exchange` class
- ✅ Write unit tests for `Order` class
- [ ] Add low-latency design to matching algorithm (MAYBE)
- [ ] Add Python code for analysis: vanilla algo vs. low-latency design algo (MAYBE)
 
## Piotr Kurek:
- ✅ Create project structure (classes, mathods, main)
- ✅ Implement logic of `OrderBook` class
- ✅ Write unit tests for `OrderBook` class
- [ ] reformat messeges + add timestamp + add trader ID to non-random int
- [ ] trade execution message in matchOrder
- [ ] Refactor code (possibly extract mathing algorithm from `Exchange` class to new `MatchingEngine` class)

## Naksh Patel:
- ✅ Implement logic of `Trader` class
- ✅ Write unit tests for `Trader` class
- [ ] Change modifyOrder in Exchange class to not resubmit, but modify on exchange
- [ ] Add exception handling
- [ ] Look into Python & C++ merge -> create analysis framework in Python

## Mohammed Ali Yasin:
- ✅ Implement logic of `Exchange` class
- ✅ Write unit tests for `Exchange` class
- [ ] Change main to perform simulation: informed vs. non-informed trading (input for Python)

## Panyu Chen:
- ✅ Implement logic of `Order` class
- ✅ Write unit tests for `Order` class

