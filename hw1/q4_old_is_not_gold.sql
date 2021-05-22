/* 1977 -> 197 -> 1970 -> 1970s*/
select cast((t.premiered / 10) * 10 as text) || 's' as decade, count(1) as c
from titles as t
where t.premiered is not null
group by t.premiered / 10 /* group by each decade*/
order by c desc;
