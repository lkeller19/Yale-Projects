import sys
import pickle
from datetime import datetime

class BankCLI:
    """User interface for bank"""

    def __init__(self):
        self._bank = Bank()
        self._choices = {
                    "open account": self._open_account,
                    "summary": self._summary,
                    "select account": self._select_account,
                    "list transactions": self._list_transactions,
                    "add transaction": self._add_transaction,
                    "<monthly triggers>": self._monthly_triggers,
                    "save": self._save,
                    "load": self._load,
                    "quit": self._quit,
                }
        
    def _display_menu(self):
        """Prints options"""

        print(
f"""Currently selected account: {self._display_selected()}
Enter command
open account, summary, select account, list transactions, add transaction, <monthly triggers>, save, load, quit"""
        )

    def run(self):
        """Display the menu and respond to choices."""

        while True:
            self._display_menu()
            choice = input(">")
            action = self._choices.get(choice)
            if action:
                action()
            else:
                print("{0} is not a valid choice".format(choice))

    def _summary(self):
        """calls summary method in Bank"""
        
        self._bank.summary()
    
    def _display_selected(self):
        """Displays selected account"""

        if self._bank.selected is None:
            return "None"
        else:
            return f"{self._bank.selected.version}#{self._bank.selected.ident:09d},\tbalance: ${self._bank.selected.amount:,.2f}"
    
    def _open_account(self):
        """prompts user for input and calls open_account() in bank"""

        print("Type of account? (checking/savings)")
        version = input(">")
        print("Initial deposit amount?")
        amount = input(">")
        self._bank.open_account(version, amount)

    def _select_account(self):
        """Prompts user for input and calls select_account() in bank"""

        self._summary()
        print("Enter account number")
        number = input(">")
        self._bank.select_account(number)

    def _list_transactions(self):
        """calls list_transactions() in bank"""

        self._bank.list_transactions()
    
    def _add_transaction(self):
        """calls add_transaction() in bank"""

        self._bank.add_transaction()

    def _monthly_triggers(self):
        """calls monthly_triggers in bank"""

        self._bank.monthly_triggers()

    def _save(self):
        """saves account data"""

        with open("bankCLI_save.pickle", "wb") as f:
            pickle.dump(self._bank, f)

    def _load(self):
        """loads previously saved account data"""

        with open("bankCLI_save.pickle", "rb") as f:
            self._bank = pickle.load(f)

    def _quit(self):
        """ends the program"""

        sys.exit(0)

class Bank:
    """Represents a bank"""

    def __init__(self):
        """Display a menu and respond to choices when run."""
        self.selected = None
        self._accounts = []
        self._num_accounts = 0

    def open_account(self, version, amount):
        """opens an account"""

        if version == "checking":
            a = CheckingAccount(.001, "Checking", amount, self._num_accounts + 1)
            self._num_accounts += 1
            self._accounts.append(a)
        elif version == "savings":
            a = SavingsAccount(.02, "Savings", amount, self._num_accounts + 1)
            self._num_accounts += 1
            self._accounts.append(a)
        else:
            pass
        self.summary()

    def summary(self):
        """gives a summary of all accounts"""

        for account in self._accounts:
                version = account.version
                ident = account.ident
                amount = account.amount
                print(f"{version}#{ident:09d},\tbalance: ${amount:,.2f}")

    def select_account(self, number):
        """Selects the desired account"""

        for account in self._accounts:
            if account.ident == int(number):
                amount = account.amount
                self.selected = account        

    def list_transactions(self):
        """Lists the transactions for the selected account"""

        for transaction in self.selected.transactions:
            print(transaction)

    def add_transaction(self):
        """Adds a transaction on the selected account"""

        print("Date? (YYYY-MM-DD)")
        date = input(">")
        print("Amount?")
        amount = input(">")
        self.selected.add_trans(date, amount)
        

    def monthly_triggers(self):
        """triggers interest and a charge if checking account is less than $100"""

        for account in self._accounts:
            t = Transaction(str(datetime.now())[0:10], account.amount * account.interest, True)
            account.transactions.append(t)
            account.amount += account.amount * account.interest

            if account.amount < 100 and account.version == "Checking":
                account.amount -= 10
                t = Transaction(str(datetime.now())[0:10], -10, True)
                account.transactions.append(t)

class Transaction:
    """Represents a transaction"""

    def __init__(self, date, amount, trigger):
        self.date = date
        self.amount = amount
        self.monthly_trigger = trigger

    def __str__(self):
        return f"{self.date}, ${float(self.amount):,.2f}"

class Account:
    """Represents a collection of accounts."""

    def __init__(self, version, amount, ident):
        """Initializes accounts with an empty list."""
        self.version = version
        self.amount = float(amount)
        self.ident = ident
        self.transactions = [Transaction(str(datetime.now())[0:10], amount, False)]   

    def add_trans(self, date, amount): #change this so it uses the super call
        """Specified in the child class"""   
        pass

class SavingsAccount(Account): #change this so it overrides str (and maybe other builtins) and user super to call the last part of add_trans
    """Represents a savings account"""

    def __init__(self, interest, *args, **kwargs):
        """Initializes a savings account"""
        self.interest = interest
        super().__init__(*args,**kwargs)


    def add_trans(self, date, amount):
        """Adds a transaction on the selected savings account"""
        daily = 0
        monthly = 0
        for transaction in self.transactions:
            if date[0:10] == transaction.date and transaction.monthly_trigger is False:
                daily += 1
            if date[5:7] == transaction.date[5:7] and transaction.monthly_trigger is False:
                if date[0:4] == transaction.date[0:4]:
                    monthly += 1

        check = self.amount + float(amount)      
        if daily < 2 and monthly < 5 and check >= 0:
            self.amount = check
            t = Transaction(date, amount, False)
            self.transactions.append(t)

class CheckingAccount(Account): #change this so it overrides str (and maybe other builtins) and user super to call add_trans
    """Represents a checking account"""

    def __init__(self, interest, *args, **kwargs):
        """Initializes a checking account"""

        self.interest = interest
        super().__init__(*args,**kwargs)

    def add_trans(self, date, amount):
        """Adds a transaction on the selected checking account"""

        check = self.amount + float(amount)    
        if check >= 0:
            self.amount = check
            t = Transaction(date, amount, False)
            self.transactions.append(t)

if __name__ == "__main__":
    BankCLI().run()