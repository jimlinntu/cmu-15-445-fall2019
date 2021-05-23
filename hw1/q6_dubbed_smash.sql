.headers on

select (select primary_title from titles where t.title_id = title_id) as ptitle, count(1) as c
from titles as t
inner join akas as a on t.title_id = a.title_id
group by t.title_id
order by c desc
limit 10;
