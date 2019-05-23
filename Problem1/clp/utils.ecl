%%
% zip(Xs, Ys, L)
%
% True if L is a list of pairs (X,Y)
% such that X is in Xs and Y is in Ys
% in order of appearance.
%
zip([], [], []).
zip([X|Xs], [Y|Ys], [v(X, Y)|L]) :-
	zip(Xs, Ys, L).

%% getIds(Ids)
%
% True if Ids is the list of all task Ids
%
getData(Ids, Durations, Workers) :-
    findall(Id, tarefa(Id, _, _, _), Ids),
    findall(Dur, tarefa(_, _, Dur, _), Durations),
    findall(Workers, tarefa(_, _, _, Workers), Workers).

%%
% myMember(L, Id, X)
%
% True if L is a list of pairs and (Id, X)
% is in L
%
myMember([v(Id, X)|_], Id, X) :- !. 
myMember([_|L], Id, St) :-
	myMember(L, Id, St).

%%
% filterById(L, Ids, Xs)
%
% True if L is a list of pairs
% and Xs is a list of X such that for each
% Id in Ids, (Id, X) is in L
%
filterById(_, [], []).
filterById(L, [Id|Ids],  [X|Xs]) :-
	myMember(L, Id, X),
	filterById(L, Ids, Xs).

%%
%
% sum_list(Xs, Sum)
%
% True if Sum is the sum of all X in Xs
%
sum_list(Xs, Sum) :- sum_list(Xs, 0, Sum).
sum_list([], Sum, Sum).
sum_list([X|Xs], Sum0, Sum) :-
    Sum1 is Sum0 + X,
    sum_list(Xs, Sum1, Sum).
