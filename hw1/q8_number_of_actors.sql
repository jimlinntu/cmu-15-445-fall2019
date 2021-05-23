.headers on

with MarkID (person_id) as (
    select p.person_id
    from people as p
    where p.name == 'Mark Hamill' and p.born == 1951
),
titles_Mark_appears_in (title_id) as (
    select c.title_id
    from crew as c, MarkID as M
    where c.person_id = M.person_id
)


/* It is possible that there are actors starring with Mark in multiple movies 
   That's why we need a distinct here
 */
select count(distinct person_id) as number_of_actors
from crew as c
where c.title_id in (
    select * from titles_Mark_appears_in
) and c.category in ('actor', 'actress')
