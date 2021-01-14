#python bfSMT.py diff1.txt

import sys, random, time
import ast

trueConstraints = []
numVars = 0
mostRecentConflict = [];

class Graph:
        
	def __init__(self, vertices): 
		self.V = vertices 
		self.graph = []

	def addEdge(self, u, v, w): 
		self.graph.append([u, v, w]) 
	
	def BellmanFord(self, src): 

		dist = [float("Inf")] * self.V 
		dist[src] = 0

		for i in range(self.V - 1): 
			for u, v, w in self.graph: 
				if dist[u] != float("Inf") and dist[u] + w < dist[v]: 
						dist[v] = dist[u] + w 

		for u, v, w in self.graph: 
				if dist[u] != float("Inf") and dist[u] + w < dist[v]: 
						return False

def parse(file):
    global numVars
    clauses = []
    for line in open(file):
        if (line.startswith('c')):
            continue
        if (line.startswith('p')):
            problemLine = line.split()
            numVars = int(problemLine[1])
            numClauses = [3]
            continue
        clause = []
        negative = False
        for x in line.split():
            clause.append(ast.literal_eval(x))
        clauses.append(clause)
    for y in clauses:
        y.pop(-1)
    return clauses, int(numVars)

##def parse():
##    global numVars
####    clauses = [[[11,0,1,9], [-5,1,4,5], [-5,0,2,10]],
####               [[-7,1,0,1], [-15,2,4,-3]],
####               [[-1,0,1,7]]]
##    clauses = [[[1,1,0,4]],
##               [[2,2,1,3]],
##               [[33,2,0,7],[3,2,0,7]]]
##    numVars = 6
##    return clauses, int(numVars)


def bcp(formula, unit):
    global trueConstraints
    if(checkConstraints(trueConstraints + [unit]) == False):
        trueConstraints.append(unit)
        return formula
    updated = []
    for clause in formula:
        conflicts = []
        containsUnit = False
        containsConflict = False
        for constraint in clause:
            if(constraint == unit):
                containsUnit = True
                continue
            if(checkConflict(unit, constraint)):
                conflicts.append(constraint)
                containsConflict = True
        if(containsUnit):
            continue
        if(containsConflict):
            newClause = [x for x in clause if x not in conflicts]
            if(len(newClause) == 0):
                return -1
            updated.append(newClause)
        else:
            updated.append(clause)
    trueConstraints.append(unit)
    return updated


def checkConflict(constraint1, constraint2):
    global mostRecentConflict;
    if(constraint1[1] == constraint2[1] and constraint1[2] == constraint2[2] and (constraint1[0] > constraint2[3] or constraint2[0] > constraint1[3])):
        return True
    else:
        mostRecentConflict = [constraint1, constraint2]
        return False

def get_counter(formula):
    counter = []
    for clause in formula:
        for constraint in clause:
            if (constraint not in counter):
                counter.append(constraint)
    return counter

def nextVariable(formula):
##    counter = get_counter(formula)
##    return random.choice(counter)
    return formula[0][0]



def unit_propagation(formula):
    global trueConstraints
    assignment = []
    unit_clauses = []
    for clause in formula:
        if len(clause) == 1:
            unit_clauses.append(clause)
    while len(unit_clauses) > 0:
        unit = unit_clauses[0]
        formula = bcp(formula, unit[0])
        assignment += [unit[0]]
        if formula == -1:
            return -1, []
        if (checkConstraints(trueConstraints) == False):
                trueConstraints.pop()
                return -1, []
        if not formula:
            return formula, assignment
        unit_clauses = [clause for clause in formula if len(clause) == 1]
    return formula, assignment


def backtracking(formula, assignment):
    global trueConstraints
    formula, unit_assignment = unit_propagation(formula)
    assignment = assignment + unit_assignment
    if (formula == -1):
        return []
    if (checkConstraints(trueConstraints) == False):
        trueConstraints.pop()
        return []
    if not formula:
        return assignment
    variable = nextVariable(formula)
    solution = backtracking(bcp(formula, variable), assignment + [variable])
    if not solution:
        for i in range(1,len(formula[0])):
            variable2 = formula[0][i]
            solution = backtracking(bcp(formula, variable2), assignment + [variable2])
            if (solution):
                break
    return solution

def checkConstraints(constraints1):
        global mostRecentConflict;
        constraints = []
        for i in constraints1:
                constraints.append(i)
        g = Graph(numVars)
        newConstraint = []
        currentConstraints = []
        for constraint in constraints:
                alreadyExists = False
                conflictExists = False
                for cc in currentConstraints:
                        if(cc[1]==constraint[1] and cc[2]==constraint[2]):
                                if(cc[0]<=constraint[3] and constraint[0]<=cc[3]):
                                        alreadyExists = True
                                        break
                                else:
                                       conflictExists = True
                                       break
                        if(cc[1]==constraint[2] and cc[2]==constraint[1]):
                                if(-cc[0]>=constraint[0] and -cc[3] <= constraint[3]):
                                        alreadyExists = True
                                        break
                                else:
                                        conflictExists = True
                                        break
                if(alreadyExists):
                        continue
                if(conflictExists):
                        mostRecentConflict = [cc,constraint]
                        return False
                g.addEdge(constraint[1], constraint[2], constraint[3])
                g.addEdge(constraint[2], constraint[1], -constraint[0])
                currentConstraints.append(constraint)
        limit = 0
                
        for i in constraints:
                for j in constraints:
                        while(limit<10):
                                if(i!=j):
                                        if(i[1] == j[1]):
                                                if(i[2] == j[2]):
                                                        if(i[0]<=j[3] and j[0]<=i[3]):
                                                                continue
                                                        else:
                                                                mostRecentConflict = [i,j]
                                                                return False
                                                else:
                                                        newConstraint = [i[0]-j[3], j[2], i[2], i[3]-j[0]]
                                        elif(i[1] == j[2]):
                                                if(i[2] == j[1]):
                                                        if(-i[0] >= j[0] and -i[3] <= j[3]):
                                                                continue
                                                        else:
                                                                mostRecentConflict = [i,j]
                                                                return False
                                                else:
                                                        newConstraint = [i[0]+j[0], j[1], i[2], i[3]+j[3]]
                                        elif(i[2] == j[1]):
                                                if(i[1] == j[2]):
                                                        if(-i[0] >= j[0] and -i[3] <= j[3]):
                                                                continue
                                                        else:
                                                                mostRecentConflict = [i,j]
                                                                return False
                                                else:
                                                        newConstraint = [-i[3]-j[3], j[2], i[1], -i[0]-j[0]]
                                        elif(i[2] == j[2]):
                                                if(i[1] == j[1]):
                                                        if(i[0]<=j[3] and j[0]<=i[3]):
                                                                continue
                                                        else:
                                                                mostRecentConflict = [i,j]
                                                                return False
                                                else:
                                                        newConstraint = [-i[3]+j[0], j[1], i[1], -i[0]+j[3]]

                                        if(newConstraint != []):
                                                for k in constraints:
                                                        if(newConstraint[1]==k[1] and newConstraint[2]==k[2]):
                                                                if(newConstraint[0] > k[3] or k[0] > newConstraint[3]):
                                                                        mostRecentConflict = [i,j]
                                                                        return False
                                                if(newConstraint not in constraints):
                                                        g.addEdge(newConstraint[1], newConstraint[2], constraint[3])
                                                        g.addEdge(newConstraint[2], newConstraint[1], -newConstraint[0])
                                                        constraints.append(newConstraint)
                                                newConstraint = []
                                limit = limit + 1
        if(g.BellmanFord(0)==False):
                return False
        return True
    
    

def main():
    start = time.time()
    clauses, numVars = parse(sys.argv[1])
    solution = backtracking(clauses, [])
    if solution:
##        solution += [x for x in clauses if x not in solution and -x not in solution] #adds numbers that aren't in the solution (set positively)
#         solution.sort(key=lambda x: abs(x)) #ensures numbers are in order     
        print ('SATISFIABLE')
        print ('' + ' '.join([str(x) for x in solution]) + ' 0') #just formatting
    else:
        print ('UNSATISFIABLE')
        print("Cause of conflict: ")
        print(mostRecentConflict)
    end = time.time()
    totalTime = end-start
    print("Time Taken: ", totalTime)


if __name__ == '__main__':
    main()
