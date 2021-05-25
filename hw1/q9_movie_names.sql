.headers on

with ids (id) as (
    select p.person_id from people as p
    where (p.name == 'Mark Hamill' and p.born == 1951) or
          (p.name == 'George Lucas' and p.born == 1944)
),
titles_they_are_in (title_id) as (
    select c.title_id
    from crew as c
    where c.person_id in (select * from ids)
    group by c.title_id
    having count(distinct c.person_id) == (select count(1) from ids)
)

select t.primary_title
from titles as t
where t.title_id in (select title_id from titles_they_are_in)
and t.type == 'movie'
order by t.primary_title asc;
