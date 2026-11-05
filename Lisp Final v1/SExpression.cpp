#include <iostream>
#include <string>
#include <variant>
#include <memory>
#include <cctype>
#include <stdexcept>

struct SExpression;

// S-Expression data types
// An atom is a symbol, such as:
// a, b, 45, test, +, -, nil
struct Atom {
    std::string symbol;
};

// Represents the empty list: ()
struct Nil {
};

// A pair contains two S-expressions:
// car = first    cdr = next
struct Pair {
    std::shared_ptr<SExpression> car;
    std::shared_ptr<SExpression> cdr;
};

// An S-expression is an Atom, Nil, or Pair.
struct SExpression {
    std::variant<Atom, Nil, Pair> value;

    SExpression(const Atom& atom)
        : value(atom)
    {
    }

    SExpression(const Nil& nil)
        : value(nil)
    {
    }

    SExpression(const Pair& pair)
        : value(pair)
    {
    }
};

// Global environment.
// rho is a list of definitions.
SExpression rho(Nil{});

// Local environment stack.
// Each environment contains two parallel lists:
// ((name1 name2 ...) (value1 value2 ...))
//
// The most recent environment is at the front.
// The stack starts empty.
SExpression localStack(Nil{});

// Lexer
std::string nextToken() {
    char ch;

    // Skip whitespace
    while (std::cin.get(ch)) {
        if (!std::isspace(static_cast<unsigned char>(ch)))
        {
            break;
        }
    }

    // EOF
    if (!std::cin) {
        return "";
    }

    // Parentheses and single quote are their own tokens
    if (ch == '(' || ch == ')' || ch == '\'') {
        return std::string(1, ch);
    }

    // Read symbol
    std::string symbol;
    symbol += ch;

    while (std::cin.get(ch)) {
        // A symbol ends at whitespace or parentheses
        if (std::isspace(static_cast<unsigned char>(ch)) || ch == '(' || ch == ')' || ch == '\'') {
            // Put the character back so the next call can process it
            std::cin.unget();
            break;
        }

        symbol += ch;
    }

    return symbol;
}

// Parser
SExpression readExpression();

// Forward declaration for cons
SExpression cons(const SExpression& first, const SExpression& rest);

// Read LIST
SExpression readList() {
    std::string token = nextToken();

    // () is the empty list
    if (token == ")") {
        return SExpression(Nil{});
    }

    // EOF before ')'
    if (token.empty()) {
        throw std::runtime_error(
            "Unexpected end of input while reading a list"
        );
    }

    // Read the first expression.
    SExpression first = (token == "(") ? readList() : (token == "'") ? readExpression() : SExpression(Atom{token});

    // If the first token was a quote, we need to construct (quote <expression>) instead.
    if (token == "'") {
        SExpression quoteAtom = SExpression(Atom{"quote"});
        SExpression emptyList = SExpression(Nil{});

        SExpression argumentList = cons(first, emptyList);

        first = cons(quoteAtom, argumentList);
    }

    // Read the rest of the list
    SExpression rest = readList();

    // Create pair cell
    Pair pair;

    pair.car = std::make_shared<SExpression>(first);
    pair.cdr = std::make_shared<SExpression>(rest);

    return SExpression(pair);
}

// Read one S-expression
SExpression readExpression() {
    std::string token = nextToken();

    if (token.empty()) {
        throw std::runtime_error("Unexpected end of input");
    }

    // '(' starts a list
    if (token == "(") {
        return readList();
    }

    // ')' cannot start an expression
    if (token == ")") {
        throw std::runtime_error("Unexpected ')'");
    }

    // A single quote is shorthand for (quote ...)
    if (token == "'") {
        SExpression quotedExpression = readExpression();

        // Build: (quote quotedExpression)
        SExpression quoteAtom = SExpression(Atom{"quote"});

        SExpression emptyList = SExpression(Nil{});

        SExpression argumentList = cons(quotedExpression, emptyList);

        return cons(quoteAtom, argumentList);
    }

    // Anything else is an atom
    return SExpression(Atom{token});
}

// Printer
void printExpression(const SExpression& expression);

/*
Print the contents of a properly terminated list.

For:

    (a b c)

the Pair structure is:

    Pair(a,
         Pair(b,
              Pair(c, Nil)))

This function prints:

    a b c

without adding the outer parentheses. */
void printListContents(const SExpression& expression) {
    const Pair* pair = std::get_if<Pair>(&expression.value);

    if (pair == nullptr) {
        return;
    }

    // Print the car.
    printExpression(*pair->car);

    // Look at the cdr.
    const SExpression& rest = *pair->cdr;

    // If cdr is Nil, the list is done.
    if (std::holds_alternative<Nil>(rest.value)) {
        return;
    }

    // If cdr is another Pair, print a space and continue through the list.
    if (std::holds_alternative<Pair>(rest.value)) {
        std::cout << " ";
        printListContents(rest);
        return;
    }

    // This would be a dotted pair.
    std::cout << " . ";
    printExpression(rest);
}

// Print a Pair.
//
// A Pair represents a list, so we put parentheses
// around its contents.
void printPair(const Pair& pair) {
    std::cout << "(";

    printExpression(*pair.car);

    const SExpression& rest = *pair.cdr;

    // The cdr is another pair, so continue printing the list.
    if (std::holds_alternative<Pair>(rest.value)) {
        std::cout << " ";
        printListContents(rest);
    }
    // The cdr is Nil, so the list is already complete.
    else if (!std::holds_alternative<Nil>(rest.value)) {
        // Dotted pair.
        std::cout << " . ";
        printExpression(rest);
    }

    std::cout << ")";
}

// Print any S-expression.
void printExpression(const SExpression& expression) {
    // Atom
    if (const Atom* atom = std::get_if<Atom>(&expression.value)) {
        std::cout << atom->symbol;
        return;
    }

    // Nil
    if (std::holds_alternative<Nil>(expression.value)) {
        std::cout << "()";
        return;
    }

    // Pair
    if (const Pair* pair = std::get_if<Pair>(&expression.value)) {
        printPair(*pair);
    }
}

// Return the car (first) of a Pair.
SExpression car(const SExpression& expression) {
    const Pair* pair = std::get_if<Pair>(&expression.value);

    if (pair == nullptr) {
        throw std::runtime_error("car requires a non-empty list");
    }

    return *pair->car;
}

// Return the cdr (next) of a Pair.
SExpression cdr(const SExpression& expression) {
    const Pair* pair = std::get_if<Pair>(&expression.value);

    if (pair == nullptr) {
        throw std::runtime_error("cdr requires a non-empty list");
    }

    return *pair->cdr;
}

// Construct a new Pair from two S-expressions.
SExpression cons(const SExpression& first, const SExpression& rest) {
    Pair pair;

    pair.car = std::make_shared<SExpression>(first);
    pair.cdr = std::make_shared<SExpression>(rest);

    return SExpression(pair);
}

// Convert an error message into an S-expression.
SExpression errorExpression(const std::string& message) {
    SExpression errorSymbol(Atom{"error"});
    SExpression errorMessage(Atom{"\"" + message + "\""});

    // Build: (error "message")
    return cons(
        errorSymbol,
        cons(errorMessage, SExpression(Nil{}))
    );
}

/*
      HELPER FUNCTIONS
---------------------------
Return true if the expression is ().
*/
bool isNil(const SExpression& expression) {
    return std::holds_alternative<Nil>(expression.value);
}

// Return true if the expression is an atom/symbol.
bool isAtom(const SExpression& expression) {
    return std::holds_alternative<Atom>(expression.value);
}

// Return true if the expression is a list/cell.
bool isList(const SExpression& expression) {
    return std::holds_alternative<Pair>(expression.value);
}

// Return true if the expression is an atom that represents a valid integer.
bool isNumber(const SExpression& expression) {
    const Atom* atom = std::get_if<Atom>(&expression.value);

    // Numbers are represented as atoms.
    if (atom == nullptr) {
        return false;
    }

    const std::string& symbol = atom->symbol;

    // An empty symbol is not a number.
    if (symbol.empty()) {
        return false;
    }

    size_t start = 0;

    // Allow an optional leading + or - sign.
    if (symbol[0] == '+' || symbol[0] == '-') {
        // A sign by itself is not an integer.
        if (symbol.length() == 1) {
            return false;
        }

        start = 1;
    }

    // Every remaining character must be a digit.
    for (size_t i = start; i < symbol.length(); ++i) {
        if (!std::isdigit(static_cast<unsigned char>(symbol[i]))) {
            return false;
        }
    }

    return true;
}

// Convert a numeric S-expression into an integer.
int expressionToInt(const SExpression& expression) {
    if (!isNumber(expression)) {
        throw std::runtime_error(
            "Expected a number"
        );
    }

    const Atom* atom = std::get_if<Atom>(&expression.value);

    try {
        return std::stoi(atom->symbol);
    }
    catch (const std::exception&) {
        throw std::runtime_error(
            "Invalid integer: " + atom->symbol
        );
    }
}

// Convert an integer into a numeric S-expression.
SExpression intToExpression(int value) {
    return SExpression(Atom{std::to_string(value)});
}

// Reusable argument-count helper functions.
// Makes it so that I wont need error handling for every function.
int listLength(const SExpression& expression) {
    int length = 0;
    SExpression current = expression;

    while (!isNil(current)) {
        if (!isList(current)) {
            throw std::runtime_error(
                "Expected a proper argument list"
            );
        }

        ++length;
        current = cdr(current);
    }

    return length;
}

void requireArguments(const std::string& functionName, const SExpression& arguments, int expected) {
    int actual = listLength(arguments);

    if (actual != expected) {
        throw std::runtime_error(
            functionName + " requires exactly " +
            std::to_string(expected) +
            (expected == 1 ? " argument" : " arguments")
        );
    }
}

// Push a new local environment onto the stack.
void pushLocalEnvironment(
    const SExpression& names,
    const SExpression& values
) {
    if (listLength(names) != listLength(values)) {
        throw std::runtime_error(
            "Local environment names and values must match"
        );
    }

    // Build: (names values)
    SExpression environment =
        cons(names, cons(values, SExpression(Nil{})));

    // Push onto the front of the stack.
    localStack = cons(environment, localStack);
}


// Remove the most recent local environment.
void popLocalEnvironment() {
    if (isNil(localStack)) {
        throw std::runtime_error(
            "Cannot pop an empty local environment stack"
        );
    }

    localStack = cdr(localStack);
}

// Lisp truth value
SExpression trueValue() {
    return SExpression(Atom{"T"});
}

// Lisp false value
SExpression falseValue() {
    return SExpression(Nil{});
}

// Look up a symbol in the local environments first then in the global environment.
SExpression lookup(const SExpression& expression) {
    const Atom* wanted = std::get_if<Atom>(&expression.value);

    // Lookup only applies to atoms.
    if (wanted == nullptr) {
        return expression;
    }

    // Search local environments, newest first.
    SExpression currentEnvironment = localStack;

    while (!isNil(currentEnvironment)) {
        // Each stack entry is:
        // ((name1 name2 ...) (value1 value2 ...))
        SExpression environment = car(currentEnvironment);

        SExpression names = car(environment);
        SExpression values = car(cdr(environment));

        // Search the parallel lists together.
        while (!isNil(names) && !isNil(values)) {
            SExpression localName = car(names);
            SExpression localValue = car(values);

            const Atom* nameAtom =
                std::get_if<Atom>(&localName.value);

            if (nameAtom != nullptr &&
                nameAtom->symbol == wanted->symbol) {
                return localValue;
            }

            names = cdr(names);
            values = cdr(values);
        }

        // Move to the next older local environment.
        currentEnvironment = cdr(currentEnvironment);
    }

    // No local match. Search the global environment.
    SExpression current = rho;

    while (!isNil(current)) {
        SExpression definition = car(current);

        SExpression name = car(definition);
        SExpression value = car(cdr(definition));

        const Atom* nameAtom =
            std::get_if<Atom>(&name.value);

        if (nameAtom != nullptr &&
            nameAtom->symbol == wanted->symbol) {
            return value;
        }

        current = cdr(current);
    }

    // Unbound symbols evaluate to themselves.
    return expression;
}

// quote simply returns its argument without evaluating it.
SExpression quote(const SExpression& expression) {
    return expression;
}

// Forward declaration for eval.
SExpression eval(const SExpression& expression);

// Evaluate each expression in an argument list.
// Return a new list containing the evaluated values.
SExpression evaluateArguments(const SExpression& arguments) {
    if (isNil(arguments)) {
        return SExpression(Nil{});
    }

    if (!isList(arguments)) {
        throw std::runtime_error(
            "Expected a proper argument list"
        );
    }

    SExpression firstValue = eval(car(arguments));

    SExpression remainingValues =
        evaluateArguments(cdr(arguments));

    return cons(firstValue, remainingValues);
}

// Evaluate an S-expression.
SExpression eval(const SExpression& expression) {
    // Atoms are looked up in the global environment then in the local environment
    if (std::holds_alternative<Atom>(expression.value)) {
        return lookup(expression);
    }

    // () evaluates to itself.
    if (std::holds_alternative<Nil>(expression.value)) {
        return expression;
    }

    const Pair* pair = std::get_if<Pair>(&expression.value);

    if (pair == nullptr) {
        throw std::runtime_error("Invalid expression");
    }

    // The first element determines the function.
    const SExpression& function = *pair->car;

    const Atom* atom = std::get_if<Atom>(&function.value);

    if (atom == nullptr) {
        throw std::runtime_error(
            "First element of expression must be an atom"
        );
    }

    std::string name = atom->symbol;

    const SExpression& arguments = *pair->cdr;

    // DEF
    if (name == "def") {
        requireArguments("def", arguments, 3);

        // Get the function name, parameter list, and body.
        SExpression functionName = car(arguments);
        SExpression parameters = car(cdr(arguments));
        SExpression body = car(cdr(cdr(arguments)));

        // The function name must be an atom.
        if (!isAtom(functionName)) {
            throw std::runtime_error(
                "def requires a symbol as its function name"
            );
        }

        // Build: (function (args) body)
        SExpression functionExpression = cons(
            SExpression(Atom{"function"}),
            cons(
                parameters,
                cons(body, SExpression(Nil{}))
            )
        );

        // Build: (set name functionExpression)
        SExpression setExpression = cons(
            SExpression(Atom{"set"}),
            cons(
                functionName,
                cons(functionExpression, SExpression(Nil{}))
            )
        );

        // Evaluate the set expression to store the function.
        return eval(setExpression);
    }

    // FUNCTION
    if (name == "function") {
        requireArguments("function", arguments, 2);

        SExpression parameters = car(arguments);

        // Parameters must be a list or nil.
        if (!isNil(parameters) && !isList(parameters)) {
            throw std::runtime_error(
                "function requires a parameter list"
            );
        }

        // Every parameter must be an atom.
        SExpression current = parameters;

        while (!isNil(current)) {
            SExpression parameter = car(current);

            if (!isAtom(parameter)) {
                throw std::runtime_error(
                    "function parameters must be symbols"
                );
            }

            current = cdr(current);
        }

        // Return the original function expression. Do not evaluate the function body.
        return expression;
    }

    // SET
    if (name == "set") {
        requireArguments("set", arguments, 2);
        
        // Do not evaluate the variable name.
        SExpression variable = car(arguments);

        // The first argument to set must be a symbol.
        if (!isAtom(variable)) {
            throw std::runtime_error(
                "set requires a symbol as its first argument"
            );
        }

        SExpression remainingArguments = cdr(arguments);
        SExpression valueExpression = car(remainingArguments);

        // Evaluate the value.
        SExpression value = eval(valueExpression);

        // Build (variable value).
        SExpression definition =
            cons(variable,
                 cons(value, SExpression(Nil{})));

        // Add it to the front of rho.
        rho = cons(definition, rho);

        // Result of set is undefined by the assignment.
        return value;
    }

    // CAR
    if (name == "car") {
        requireArguments("car", arguments, 1);
        
        SExpression argument = car(arguments);
        return car(eval(argument));
    }

    // CDR
    if (name == "cdr") {
        requireArguments("cdr", arguments, 1);
        
        SExpression argument = car(arguments);
        return cdr(eval(argument));
    }

    // CONS
    if (name == "cons") {
        requireArguments("cons", arguments, 2);
        
        SExpression firstArgument = car(arguments);

        SExpression remainingArguments = cdr(arguments);
        SExpression secondArgument = car(remainingArguments);

        return cons(
            eval(firstArgument),
            eval(secondArgument)
        );
    }

    // QUOTE
    if (name == "quote") {
        requireArguments("quote", arguments, 1);

        // quote does NOT evaluate its argument.
        return quote(car(arguments));
    }

    // EVAL
    if (name == "eval") {
        requireArguments("eval", arguments, 1);
        
        SExpression argument = car(arguments);
        return eval(argument);
    }

    // NIL?
    if (name == "nil?") {
        requireArguments("nil?", arguments, 1);

        SExpression argument = car(arguments);
        SExpression value = eval(argument);

        if (isNil(value)) {
            return trueValue();
        }

        return falseValue();
    }

    // ATOM?
    if (name == "atom?") {
        requireArguments("atom?", arguments, 1);
        
        SExpression argument = car(arguments);
        SExpression value = eval(argument);

        if (isAtom(value)) {
            return trueValue();
        }

        return falseValue();
    }

    // LIST?
    if (name == "list?") {
        requireArguments("list?", arguments, 1);
        
        SExpression argument = car(arguments);
        SExpression value = eval(argument);

        if (isList(value)) {
            return trueValue();
        }

        return falseValue();
    }

    // NOT?
    if (name == "not?") {
        requireArguments("not?", arguments, 1);
        
        SExpression argument = car(arguments);
        SExpression value = eval(argument);

        if (isNil(value)) {
            return trueValue();
        }

        return falseValue();
    }

    // NUMBER?
    if (name == "number?") {
        requireArguments("number?", arguments, 1);
        
        SExpression argument = car(arguments);
        SExpression value = eval(argument);

        if (isNumber(value)) {
            return trueValue();
        }

        return falseValue();
    }

    //----BOOLEAN LOGIC----
    // AND?
    if (name == "and?") {
        requireArguments("and?", arguments, 2);
        
        SExpression firstArgument = car(arguments);
        SExpression firstValue = eval(firstArgument);

        // Short circuit if first argument is false.
        if (isNil(firstValue)) {
            return falseValue();
        }

        SExpression remainingArguments = cdr(arguments);
        SExpression secondArgument = car(remainingArguments);
        SExpression secondValue = eval(secondArgument);

        if (isNil(secondValue)) {
            return falseValue();
        }

        return trueValue();
    }

    // OR?
    if (name == "or?") {
        requireArguments("or?", arguments, 2);
        
        SExpression firstArgument = car(arguments);
        SExpression firstValue = eval(firstArgument);

        // Short circuit if first argument is true.
        if (!isNil(firstValue)) {
            return trueValue();
        }

        SExpression remainingArguments = cdr(arguments);
        SExpression secondArgument = car(remainingArguments);
        SExpression secondValue = eval(secondArgument);

        if (!isNil(secondValue)) {
            return trueValue();
        }

        return falseValue();
    }

    // EQ?
    if (name == "eq?") {
        requireArguments("eq?", arguments, 2);
        
        SExpression firstArgument = car(arguments);

        SExpression remainingArguments = cdr(arguments);
        SExpression secondArgument = car(remainingArguments);

        SExpression firstValue = eval(firstArgument);
        SExpression secondValue = eval(secondArgument);

        const Atom* firstAtom = std::get_if<Atom>(&firstValue.value);
        const Atom* secondAtom = std::get_if<Atom>(&secondValue.value);

        if (firstAtom == nullptr || secondAtom == nullptr) {
            return falseValue();
        }

        if (firstAtom->symbol == secondAtom->symbol) {
            return trueValue();
        }

        return falseValue();
    }

    // IF
    if (name == "if") {
        requireArguments("if", arguments, 3);
        
        SExpression condition = car(arguments);

        SExpression remainingArguments = cdr(arguments);
        SExpression trueExpression = car(remainingArguments);

        remainingArguments = cdr(remainingArguments);
        SExpression falseExpression = car(remainingArguments);

        SExpression conditionValue = eval(condition);

        if (isNil(conditionValue)) {
            return eval(falseExpression);
        }

        return eval(trueExpression);
    }

    // COND
    if (name == "cond") {
        requireArguments("cond", arguments, 1);

        SExpression clauses = car(arguments);

        // cond's argument must be a list or an empty list.
        if (!isNil(clauses) && !isList(clauses)) {
            throw std::runtime_error(
                "cond requires a list of condition/result pairs"
            );
        }

        if (listLength(clauses) % 2 != 0) {
            throw std::runtime_error(
                "cond requires condition/result pairs"
            );
        }

        while (!isNil(clauses)) {
            SExpression condition = car(clauses);
            clauses = cdr(clauses);

            // Every condition must have a corresponding result.
            if (isNil(clauses)) {
                throw std::runtime_error(
                    "cond condition is missing its result expression"
                );
            }

            SExpression resultExpression = car(clauses);
            clauses = cdr(clauses);

            SExpression conditionValue = eval(condition);

            if (!isNil(conditionValue)) {
                // First true condition wins.
                return eval(resultExpression);
            }
        }

        // Behavior when all conditions are nil is unspecified by the assignment. Return nil.
        return falseValue();
    }

    // ADD
    if (name == "add") {
        requireArguments("add", arguments, 2);

        SExpression firstArgument = car(arguments);
        SExpression secondArgument = car(cdr(arguments));

        SExpression firstValue = eval(firstArgument);
        SExpression secondValue = eval(secondArgument);

        int a = expressionToInt(firstValue);
        int b = expressionToInt(secondValue);

        return intToExpression(a + b);
    }

    // SUB
    if (name == "sub") {
        requireArguments("sub", arguments, 2);

        SExpression firstArgument = car(arguments);
        SExpression secondArgument = car(cdr(arguments));

        SExpression firstValue = eval(firstArgument);
        SExpression secondValue = eval(secondArgument);

        int a = expressionToInt(firstValue);
        int b = expressionToInt(secondValue);

        return intToExpression(a - b);
    }

    // MUL
    if (name == "mul") {
        requireArguments("mul", arguments, 2);

        SExpression firstArgument = car(arguments);
        SExpression secondArgument = car(cdr(arguments));

        SExpression firstValue = eval(firstArgument);
        SExpression secondValue = eval(secondArgument);

        int a = expressionToInt(firstValue);
        int b = expressionToInt(secondValue);

        return intToExpression(a * b);
    }

    // DIV
    if (name == "div") {
        requireArguments("div", arguments, 2);

        SExpression firstArgument = car(arguments);
        SExpression secondArgument = car(cdr(arguments));

        SExpression firstValue = eval(firstArgument);
        SExpression secondValue = eval(secondArgument);

        int a = expressionToInt(firstValue);
        int b = expressionToInt(secondValue);

        if (b == 0) {
            throw std::runtime_error(
                "div cannot divide by zero"
            );
        }

        return intToExpression(a / b);
    }

    // REM
    if (name == "rem") {
        requireArguments("rem", arguments, 2);

        SExpression firstArgument = car(arguments);
        SExpression secondArgument = car(cdr(arguments));

        SExpression firstValue = eval(firstArgument);
        SExpression secondValue = eval(secondArgument);

        int a = expressionToInt(firstValue);
        int b = expressionToInt(secondValue);

        if (b == 0) {
            throw std::runtime_error(
                "rem cannot divide by zero"
            );
        }

        return intToExpression(a % b);
    }

    // LT (less than)
    if (name == "lt") {
        requireArguments("lt", arguments, 2);

        SExpression firstArgument = car(arguments);
        SExpression secondArgument = car(cdr(arguments));

        SExpression firstValue = eval(firstArgument);
        SExpression secondValue = eval(secondArgument);

        int a = expressionToInt(firstValue);
        int b = expressionToInt(secondValue);

        if (a < b) {
            return trueValue();
        }

        return falseValue();
    }

    // USER-DEFINED FUNCTION CALL
    SExpression functionValue = lookup(function);

    // Check whether the lookup returned a list.
    if (isList(functionValue)) {
        SExpression functionType = car(functionValue);

        const Atom* functionAtom = std::get_if<Atom>(&functionType.value);

        // Check whether the expression begins with "function".
        if (functionAtom != nullptr && functionAtom->symbol == "function") {
            // Validate the stored function expression.
            SExpression functionArguments = cdr(functionValue);

            requireArguments("function", functionArguments, 2);

            // Get the formal parameter names.
            SExpression parameters = car(functionArguments);

            // Get the function body.
            SExpression body = car(cdr(functionArguments));

            // Evaluate the actual arguments in the caller's environment.
            SExpression actualValues = evaluateArguments(arguments);

            // Make sure parameter and argument counts match.
            if (listLength(parameters) != listLength(actualValues)) {
                throw std::runtime_error(
                    name + " called with incorrect number of arguments"
                );
            }

            // Push the new local environment.
            pushLocalEnvironment(parameters, actualValues);

            // Evaluate the function body.
            SExpression result(Nil{});

            try {
                result = eval(body);
            }
            catch (...) {
                popLocalEnvironment();
                throw;
            }
            
            popLocalEnvironment();

            return result;
        }
    }

    throw std::runtime_error("Unknown function: " + name);
}

int main() {
    while (true) {
        std::cin >> std::ws;

        if (std::cin.peek() == EOF) {
            break;
        }

        try {
            // Read the next Lisp expression.
            SExpression expression = readExpression();

            // Evaluate the expression.
            SExpression result = eval(expression);

            // Print the result.
            printExpression(result);
            std::cout << '\n';
        }
        catch (const std::exception& e) {
            // Convert the error into an S-expression.
            SExpression error = errorExpression(e.what());

            printExpression(error);
            std::cout << '\n';
        }
    }

    return 0;
}