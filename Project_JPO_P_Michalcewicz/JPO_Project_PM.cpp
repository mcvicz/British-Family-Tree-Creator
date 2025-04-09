/*
-------------------------------------------------------------------------
Family Tree Creator Project
-------------------------------------------------------------------------
University: AGH University of Krakow
Major: Elektronika i Telekomunikacja (Electronics and Telecommunication)
Class: Języki Programowania Obiektowego JPO (Object-oriented programming languages)
Professor: Rafał Frączek
-------------------------------------------------------------------------
Author: Paweł Michalcewicz
-------------------------------------------------------------------------

Description:
    This program manages a British Royal Family Tree from Queen Victoria
    down to Queen Elizabeth II’s children. Users can:
    - Add new persons (with birth/death years)
    - Print the entire family tree
    - Save all changes to a file (family_tree.dat)
    - Load existing data from file automatically on startup
    - Restore to default data (discarding any modifications)
    - Quit with or without saving
    - Use 'back' to return from submenus; use 'exit' at any prompt to terminate.
*/

#include <iostream>
#include <string>
#include <vector>
#include <limits>
#include <queue>
#include <fstream>
#include <stdexcept>
#include <cctype>   // for isdigit()
#include <cstdlib>  // for std::exit()

/*
 * TreeEntity
 * ----------
 * Abstract base class (interface) for any entity in the tree.
 * Currently, only 'Person' inherits from it, but this demonstrates
 * inheritance and virtual functions.
 */
class TreeEntity {
public:
    virtual ~TreeEntity() = default;
    virtual std::string getName() const = 0;
};

/*
 * Person
 * ------
 * Represents an individual in the family tree.
 * Inherits from TreeEntity to demonstrate interfaces.
 */
class Person : public TreeEntity {
private:
    std::string name;
    int birthYear;
    int deathYear;
    std::vector<int> children; // Holds indices of child Persons in the FamilyTree

public:
    // Constructor with optional deathYear (defaults to -1 indicating alive)
    Person() : name("Unknown"), birthYear(0), deathYear(-1) {}
    Person(const std::string& p_name, int p_birthYear, int p_deathYear = -1)
        : name(p_name), birthYear(p_birthYear), deathYear(p_deathYear) {}

    // Required override from TreeEntity
    std::string getName() const override { return name; }

    // Additional getters
    int getBirthYear() const { return birthYear; }
    int getDeathYear() const { return deathYear; }
    const std::vector<int>& getChildren() const { return children; }

    // Setters
    void setName(const std::string& newName) { name = newName; }
    void setBirthYear(int newBirthYear) { birthYear = newBirthYear; }
    void setDeathYear(int newDeathYear) { deathYear = newDeathYear; }

    // Adds a child's index to this person's children vector
    void addChild(int childIndex) {
        children.push_back(childIndex);
    }
};

/*
 * FamilyTree
 * ----------
 * Manages a collection of Person objects, including:
 * - Storing them in a vector
 * - Connecting parent/child relationships by index
 * - Loading/saving to file
 * - Printing the hierarchy with recursion
 * - BFS-based generation grouping
 */
class FamilyTree {
private:
    std::vector<Person> people; // The main container of Person objects

    /*
     * printPerson (recursive)
     * -----------------------
     * Helper function to print a Person's info and recursively print the children.
     *   index      : which Person in the 'people' vector
     *   prefix     : indentation/bar prefix for tree printing
     *   isLast     : true if this child is the last among siblings (affects how we draw lines)
     *   generation : numeric generation label (root is 1)
     */
    void printPerson(int index, const std::string& prefix, bool isLast, int generation) const {
        if (index < 0 || index >= static_cast<int>(people.size())) {
            return;
        }

        // Print the appropriate prefix for the tree lines
        std::cout << prefix;
        if (!prefix.empty()) {
            std::cout << (isLast ? "\\---" : "|---");
        }

        // Retrieve the Person
        const Person& p = people[index];
        // Print generation, name, birth and death
        std::cout << " [Gen " << generation << "] "
            << p.getName() << " (b. " << p.getBirthYear();
        if (p.getDeathYear() != -1) {
            std::cout << ", d. " << p.getDeathYear();
        }
        std::cout << ")\n";

        // Recursively print children
        const auto& kids = p.getChildren();
        if (!kids.empty()) {
            std::string newPrefix = prefix + (isLast ? "   " : "|  ");
            for (size_t i = 0; i < kids.size(); ++i) {
                bool childIsLast = (i == kids.size() - 1);
                printPerson(kids[i], newPrefix, childIsLast, generation + 1);
            }
        }
    }

public:
    /*
     * FamilyTree constructor
     * ----------------------
     * Tries to load data from "family_tree.dat".
     * If not found or invalid, it initializes the default British Royal data.
     */
    FamilyTree() {
        try {
            loadFromFile("family_tree.dat");
            std::cout << "[Data loaded from 'family_tree.dat' successfully.]\n\n";
        }
        catch (const std::exception& ex) {
            std::cerr << "[Warning] Could not load file: " << ex.what()
                << "\n[Initializing default British Royal data...]\n\n";
            initSampleFamily();
        }
    }

    /*
     * resetToDefault
     * --------------
     * Clears the current family data and re-initializes it
     * with the default British Royal data.
     */
    void resetToDefault() {
        people.clear();
        initSampleFamily();
        std::cout << "[All custom changes discarded. Restored default data.]\n";
    }

    /*
     * size()
     * ------
     * Returns how many Person objects are stored.
     */
    int size() const {
        return static_cast<int>(people.size());
    }

    /*
     * getPerson(index)
     * ----------------
     * Returns a reference to the Person at 'index'.
     * (Throws std::out_of_range if invalid.)
     */
    const Person& getPerson(int index) const {
        return people.at(index);
    }

    /*
     * addPerson
     * ---------
     * Creates a new Person with the given data, appends to 'people', and returns the index.
     */
    int addPerson(const std::string& name, int birthYear, int deathYear = -1) {
        Person p(name, birthYear, deathYear);
        people.push_back(p);
        return static_cast<int>(people.size()) - 1;
    }

    /*
     * connectParentChild
     * ------------------
     * Makes 'childIndex' a child of 'parentIndex' if both are valid.
     */
    void connectParentChild(int parentIndex, int childIndex) {
        if (parentIndex >= 0 && parentIndex < static_cast<int>(people.size()) &&
            childIndex >= 0 && childIndex < static_cast<int>(people.size())) {
            people[parentIndex].addChild(childIndex);
        }
    }

    /*
     * printFamilyTree
     * ---------------
     * Recursively prints the tree from the root Person at 'rootIndex' (Gen 1).
     */
    void printFamilyTree(int rootIndex) const {
        if (rootIndex < 0 || rootIndex >= static_cast<int>(people.size())) {
            std::cout << "[Invalid root index: " << rootIndex << "]\n";
            return;
        }
        printPerson(rootIndex, "", true, 1);
    }

    /*
     * getGenerations
     * --------------
     * Performs a BFS starting at 'rootIndex' and groups Person indices by generation/layer.
     * Returns a vector such that:
     *    result[g] = list of Person indices at generation g (0-based internally).
     */
    std::vector<std::vector<int>> getGenerations(int rootIndex) const {
        std::vector<std::vector<int>> result;
        if (rootIndex < 0 || rootIndex >= static_cast<int>(people.size())) {
            return result;
        }

        std::vector<bool> visited(people.size(), false);
        std::queue<std::pair<int, int>> q;
        q.push({ rootIndex, 0 });    // generation=0 for the root
        visited[rootIndex] = true;

        while (!q.empty()) {
            auto [curr, gen] = q.front();
            q.pop();

            if (gen >= static_cast<int>(result.size())) {
                result.resize(gen + 1);
            }
            result[gen].push_back(curr);

            // Enqueue children with generation+1
            for (int childIdx : people[curr].getChildren()) {
                if (!visited[childIdx]) {
                    visited[childIdx] = true;
                    q.push({ childIdx, gen + 1 });
                }
            }
        }
        return result;
    }

    /*
     * saveToFile
     * ----------
     * Writes all Person data (and child links) to a file in a simple text format.
     */
    void saveToFile(const std::string& filename) const {
        std::ofstream outFile(filename);
        if (!outFile) {
            throw std::runtime_error("Failed to open file for saving: " + filename);
        }

        // First line: number of Person objects
        outFile << people.size() << "\n";
        // For each Person: name, birthYear, deathYear, numberOfChildren, childIndices...
        for (const auto& p : people) {
            // Safely write name (replace newlines if any)
            std::string sanitizedName = p.getName();
            for (char& ch : sanitizedName) {
                if (ch == '\n') ch = ' ';
            }
            outFile << sanitizedName << "\n"
                << p.getBirthYear() << "\n"
                << p.getDeathYear() << "\n"
                << p.getChildren().size() << "\n";
            for (int c : p.getChildren()) {
                outFile << c << " ";
            }
            outFile << "\n";
        }
    }

    /*
     * loadFromFile
     * Attempts to read Person data from the given file. On success, the internal
     * 'people' vector is replaced with data from the file. Throws an exception
     * if the file is missing or the format is invalid.
     */
    void loadFromFile(const std::string& filename) {
        std::ifstream inFile(filename);
        if (!inFile) {
            throw std::runtime_error("File not found or cannot open: " + filename);
        }

        people.clear();

        size_t count = 0;
        inFile >> count;
        inFile.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        if (!inFile.good()) {
            throw std::runtime_error("Invalid file format (cannot read count).");
        }

        // Prepare to store child indices (read them first, then connect later)
        people.reserve(count);
        std::vector<std::vector<int>> childrenIndices(count);

        for (size_t i = 0; i < count; i++) {
            std::string name;
            std::getline(inFile, name); // person's name

            int birth = 0;
            inFile >> birth;
            inFile.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

            int death = 0;
            inFile >> death;
            inFile.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

            size_t childCount = 0;
            inFile >> childCount;
            inFile.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

            std::vector<int> tmpChildren;
            tmpChildren.reserve(childCount);

            for (size_t c = 0; c < childCount; c++) {
                int idx = -1;
                inFile >> idx;
                tmpChildren.push_back(idx);
            }
            inFile.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

            if (!inFile.good() && !inFile.eof()) {
                throw std::runtime_error("Corrupt data while reading Person #"
                    + std::to_string(i));
            }

            Person p(name, birth, death);
            people.push_back(p);

            childrenIndices[i] = tmpChildren;
        }

        // Connect parent->children
        for (size_t i = 0; i < count; i++) {
            for (int childIdx : childrenIndices[i]) {
                if (childIdx >= 0 && childIdx < (int)count) {
                    people[i].addChild(childIdx);
                }
            }
        }

        if (!inFile.good() && !inFile.eof()) {
            throw std::runtime_error("Unexpected file format error while parsing data.");
        }
    }

    /*
     * initSampleFamily
     * ----------------
     * If loading from file fails or the file doesn't exist, we create a default
     * British Royal Family from Queen Victoria -> Elizabeth II's children.
     */
    void initSampleFamily() {
        // Minimal snippet focusing on relevant ancestry
        Person queenVictoria("Queen Victoria", 1819, 1901);
        Person princeAlbert("Prince Albert of Saxe-Coburg and Gotha", 1819, 1861);

        Person edwardVII("King Edward VII", 1841, 1910);
        Person alexandraDenmark("Alexandra of Denmark", 1844, 1925);

        Person georgeV("King George V", 1865, 1936);
        Person maryTeck("Queen Mary of Teck", 1867, 1953);

        Person edwardVIII("King Edward VIII (Duke of Windsor)", 1894, 1972);
        Person wallisSimpson("Wallis Simpson, Duchess of Windsor", 1896, 1986);

        Person georgeVI("King George VI", 1895, 1952);
        Person elizabethBowes("Elizabeth Bowes-Lyon (Queen Mother)", 1900, 2002);

        Person elizabethII("Queen Elizabeth II", 1926, 2022);
        Person philipDuke("Prince Philip, Duke of Edinburgh", 1921, 2021);
        Person princessMargaret("Princess Margaret, Countess of Snowdon", 1930, 2002);

        Person kingCharlesIII("King Charles III", 1948, -1);
        Person princessDiana("Diana, Princess of Wales", 1961, 1997);
        Person queenCamilla("Queen Camilla", 1947, -1);

        Person princessAnne("Anne, Princess Royal", 1950, -1);
        Person princeAndrew("Prince Andrew, Duke of York", 1960, -1);
        Person princeEdward("Prince Edward, Duke of Edinburgh", 1964, -1);

        // Add them to the vector in order
        int victoria_Idx = addPerson(queenVictoria.getName(),
            queenVictoria.getBirthYear(),
            queenVictoria.getDeathYear());
        int albert_Idx = addPerson(princeAlbert.getName(),
            princeAlbert.getBirthYear(),
            princeAlbert.getDeathYear());

        int edwardVII_Idx = addPerson(edwardVII.getName(),
            edwardVII.getBirthYear(),
            edwardVII.getDeathYear());
        int alexandra_Idx = addPerson(alexandraDenmark.getName(),
            alexandraDenmark.getBirthYear(),
            alexandraDenmark.getDeathYear());

        int georgeV_Idx = addPerson(georgeV.getName(),
            georgeV.getBirthYear(),
            georgeV.getDeathYear());
        int maryTeck_Idx = addPerson(maryTeck.getName(),
            maryTeck.getBirthYear(),
            maryTeck.getDeathYear());

        int edwardVIII_Idx = addPerson(edwardVIII.getName(),
            edwardVIII.getBirthYear(),
            edwardVIII.getDeathYear());
        int wallis_Idx = addPerson(wallisSimpson.getName(),
            wallisSimpson.getBirthYear(),
            wallisSimpson.getDeathYear());

        int georgeVI_Idx = addPerson(georgeVI.getName(),
            georgeVI.getBirthYear(),
            georgeVI.getDeathYear());
        int elizBowes_Idx = addPerson(elizabethBowes.getName(),
            elizabethBowes.getBirthYear(),
            elizabethBowes.getDeathYear());

        int elizII_Idx = addPerson(elizabethII.getName(),
            elizabethII.getBirthYear(),
            elizabethII.getDeathYear());
        int philip_Idx = addPerson(philipDuke.getName(),
            philipDuke.getBirthYear(),
            philipDuke.getDeathYear());
        int margaret_Idx = addPerson(princessMargaret.getName(),
            princessMargaret.getBirthYear(),
            princessMargaret.getDeathYear());

        int charles_Idx = addPerson(kingCharlesIII.getName(),
            kingCharlesIII.getBirthYear(),
            kingCharlesIII.getDeathYear());
        int diana_Idx = addPerson(princessDiana.getName(),
            princessDiana.getBirthYear(),
            princessDiana.getDeathYear());
        int camilla_Idx = addPerson(queenCamilla.getName(),
            queenCamilla.getBirthYear(),
            queenCamilla.getDeathYear());

        int anne_Idx = addPerson(princessAnne.getName(),
            princessAnne.getBirthYear(),
            princessAnne.getDeathYear());
        int andrew_Idx = addPerson(princeAndrew.getName(),
            princeAndrew.getBirthYear(),
            princeAndrew.getDeathYear());
        int edward_Idx = addPerson(princeEdward.getName(),
            princeEdward.getBirthYear(),
            princeEdward.getDeathYear());

        // Connect them as parents->children
        connectParentChild(victoria_Idx, edwardVII_Idx);
        connectParentChild(albert_Idx, edwardVII_Idx);

        connectParentChild(edwardVII_Idx, georgeV_Idx);
        connectParentChild(alexandra_Idx, georgeV_Idx);

        connectParentChild(georgeV_Idx, edwardVIII_Idx);
        connectParentChild(maryTeck_Idx, edwardVIII_Idx);
        connectParentChild(georgeV_Idx, georgeVI_Idx);
        connectParentChild(maryTeck_Idx, georgeVI_Idx);

        connectParentChild(georgeVI_Idx, elizII_Idx);
        connectParentChild(elizBowes_Idx, elizII_Idx);
        connectParentChild(georgeVI_Idx, margaret_Idx);
        connectParentChild(elizBowes_Idx, margaret_Idx);

        connectParentChild(elizII_Idx, charles_Idx);
        connectParentChild(philip_Idx, charles_Idx);
        connectParentChild(elizII_Idx, anne_Idx);
        connectParentChild(philip_Idx, anne_Idx);
        connectParentChild(elizII_Idx, andrew_Idx);
        connectParentChild(philip_Idx, andrew_Idx);
        connectParentChild(elizII_Idx, edward_Idx);
        connectParentChild(philip_Idx, edward_Idx);

        // Charles + Diana, Camilla
        connectParentChild(charles_Idx, diana_Idx);
        connectParentChild(charles_Idx, camilla_Idx);
    }
};

/*
 * checkExitCommand
 * ----------------
 * If the user types 'exit' or 'EXIT' at any input prompt, we terminate
 * immediately. This ensures we can exit from submenus or mid-prompts.
 */
void checkExitCommand(const std::string& input) {
    if (input == "exit" || input == "EXIT") {
        std::cout << "[Exiting program on user request.]\n";
        std::exit(0);
    }
}

/*
 * isNumeric
 * ---------
 * Returns true if the string is purely digits or exactly "-1" (for 'still alive').
 * Used to validate user numeric input. Minimizes infinite loops on bad input.
 */
bool isNumeric(const std::string& s) {
    if (s == "-1") return true; // specifically allow -1
    if (s.empty()) return false;
    for (char c : s) {
        if (!std::isdigit(static_cast<unsigned char>(c))) {
            return false;
        }
    }
    return true;
}

/*
 * main
 * ----
 * The entry point with a text-based menu. Demonstrates:
 *  1) Add a new Person
 *  2) Print the Family Tree
 *  3) Save & Quit
 *  4) Just Quit
 *  5) Restore to Default
 *
 * 'back' and 'exit' are also recognized in submenus to go back or fully terminate.
 */
int main() {
    std::cout << "British Royal Family Tree Creator\n\n";

    FamilyTree tree;  // Will attempt to load from file, else init default
    int BFS_ROOT_INDEX = 0;  // We treat the 0th Person (Queen Victoria) as root

    while (true) {
        std::cout << "------------------------------------------\n";
        std::cout << "Main Menu (type 'exit' to terminate):\n";
        std::cout << "  1) Add a new Person\n";
        std::cout << "  2) Print the Family Tree\n";
        std::cout << "  3) Save & Quit\n";
        std::cout << "  4) Just Quit\n";
        std::cout << "  5) Restore to Default\n";
        std::cout << "------------------------------------------\n";
        std::cout << "Your choice: ";

        std::string menuInput;
        std::getline(std::cin, menuInput);
        checkExitCommand(menuInput);

        if (menuInput == "1") {
            // Add a new Person
            std::cout << "\n[Add Person - type 'exit' to quit, 'back' to return.]\n";

            // BFS generations to pick a parent
            auto generations = tree.getGenerations(BFS_ROOT_INDEX);
            if (generations.empty()) {
                std::cout << "No valid root or empty tree! Cannot add.\n";
                continue;
            }

            // Show how many generations
            std::cout << "We have " << generations.size()
                << " generation(s) under index " << BFS_ROOT_INDEX << ".\n";
            for (size_t g = 0; g < generations.size(); g++) {
                std::cout << "  Generation #" << (g + 1)
                    << " has " << generations[g].size() << " person(s).\n";
            }

            std::string genChoiceStr;
            while (true) {
                // Prompt user to pick generation (1-based)
                std::cout << "Which generation is the parent in? (1 to "
                    << generations.size() << ", 'back' to menu): ";
                std::getline(std::cin, genChoiceStr);
                checkExitCommand(genChoiceStr);
                if (genChoiceStr == "back") {
                    break; // go back to main menu
                }
                if (!isNumeric(genChoiceStr)) {
                    std::cout << "[Invalid input: must be a number or 'back'.]\n";
                    continue;
                }
                int genChoice = std::stoi(genChoiceStr) - 1; // convert to 0-based
                if (genChoice < 0 || genChoice >= static_cast<int>(generations.size())) {
                    std::cout << "[Invalid generation index!]\n";
                    continue;
                }

                auto& genList = generations[genChoice];
                if (genList.empty()) {
                    std::cout << "[That generation is empty. Cannot pick a parent.]\n";
                    break;
                }

                // Show all persons in that generation
                std::cout << "\n--- Members in Generation #" << (genChoice + 1) << " ---\n";
                for (size_t i = 0; i < genList.size(); i++) {
                    int idx = genList[i];
                    const Person& p = tree.getPerson(idx);
                    std::cout << "  (" << i + 1 << ") " << p.getName()
                        << " (b. " << p.getBirthYear();
                    if (p.getDeathYear() != -1) {
                        std::cout << ", d. " << p.getDeathYear();
                    }
                    std::cout << ")\n";
                }
                std::cout << "------------------------------------------\n";

                // Prompt user to pick which parent in that generation
                while (true) {
                    std::cout << "Pick the parent number (1 to "
                        << genList.size() << ", or 'back'): ";
                    std::string parentChoiceStr;
                    std::getline(std::cin, parentChoiceStr);
                    checkExitCommand(parentChoiceStr);
                    if (parentChoiceStr == "back") {
                        break; // go back to generation selection
                    }
                    if (!isNumeric(parentChoiceStr)) {
                        std::cout << "[Please enter a valid number or 'back'.]\n";
                        continue;
                    }
                    int parentNum = std::stoi(parentChoiceStr) - 1;
                    if (parentNum < 0 || parentNum >= static_cast<int>(genList.size())) {
                        std::cout << "[Invalid choice.]\n";
                        continue;
                    }

                    int parentIndex = genList[parentNum];

                    // Now gather data for the new Person
                    std::string childName;
                    std::string birthYearStr;
                    std::string deathYearStr;

                    // Child's name
                    std::cout << "\nEnter new person's name (or 'exit'/'back'): ";
                    std::getline(std::cin, childName);
                    checkExitCommand(childName);
                    if (childName == "back") {
                        break;
                    }

                    // Child's birth year
                    while (true) {
                        std::cout << "Enter birth year (or 'exit'/'back'): ";
                        std::getline(std::cin, birthYearStr);
                        checkExitCommand(birthYearStr);
                        if (birthYearStr == "back") {
                            break;
                        }
                        if (!isNumeric(birthYearStr)) {
                            std::cout << "[Please enter a numeric birth year.]\n";
                            continue;
                        }
                        break;
                    }
                    if (birthYearStr == "back") {
                        break;
                    }
                    int childBirth = std::stoi(birthYearStr);

                    // Child's death year
                    while (true) {
                        std::cout << "Enter death year (-1 if still alive) (or 'exit'/'back'): ";
                        std::getline(std::cin, deathYearStr);
                        checkExitCommand(deathYearStr);
                        if (deathYearStr == "back") {
                            break;
                        }
                        if (!isNumeric(deathYearStr)) {
                            std::cout << "[Please enter a numeric death year or -1.]\n";
                            continue;
                        }
                        break;
                    }
                    if (deathYearStr == "back") {
                        break;
                    }
                    int childDeath = std::stoi(deathYearStr);

                    // Create new Person in the tree
                    int newIndex = tree.addPerson(childName, childBirth, childDeath);
                    // Connect to chosen parent
                    tree.connectParentChild(parentIndex, newIndex);

                    // Confirm
                    const Person& newP = tree.getPerson(newIndex);
                    std::cout << "\n[New Person Added]\n";
                    std::cout << "   " << newP.getName()
                        << " (b. " << newP.getBirthYear();
                    if (newP.getDeathYear() != -1) {
                        std::cout << ", d. " << newP.getDeathYear();
                    }
                    std::cout << ")\n\n";

                    // Print updated family tree
                    std::cout << "Updated Family Tree\n";
                    tree.printFamilyTree(BFS_ROOT_INDEX);
                    std::cout << "===========================\n\n";
                    break; // done adding this child
                }
                break; // done with generation choice
            }
        }
        else if (menuInput == "2") {
            // Print the entire Family Tree from BFS_ROOT_INDEX
            std::cout << "\nCurrent Family Tree\n";
            tree.printFamilyTree(BFS_ROOT_INDEX);
            std::cout << "===================\n\n";
        }
        else if (menuInput == "3") {
            // Save and Quit
            try {
                tree.saveToFile("family_tree.dat");
                std::cout << "[Data saved to 'family_tree.dat'. Exiting...]\n";
            }
            catch (const std::exception& ex) {
                std::cerr << "[Error saving file: " << ex.what() << "]\n";
            }
            break;
        }
        else if (menuInput == "4") {
            // Just Quit (no save)
            std::cout << "[Exiting without saving changes.]\n";
            break;
        }
        // Added Option 5: Restore to Default
        else if (menuInput == "5") {
            std::cout << "\n[Restoring default data. All custom changes will be LOST unless you save afterward.]\n";
            tree.resetToDefault();
        }
        else {
            // Invalid menu choice
            std::cout << "[Invalid option. Please choose 1-5 or type 'exit'.]\n";
        }
    }

    std::cout << "\nProgram Finished\n";
    return 0;
}