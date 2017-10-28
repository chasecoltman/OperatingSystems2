/*
* Elevator Look
*/
#include <linux/blkdev.h>
#include <linux/elevator.h>
#include <linux/bio.h>
#include <linux/module.h>
#include <linux/slab.h>
#include <linux/init.h>

struct sstf_data{
	struct list_head queue;
	int direction;
}

//done
static void look_merged_requests (struct request_queue *q, struct request *rq, struct request *next)
{
    list_del_init(&next->queuelist);
}

//Needs to be done
static int look_dispatch(struct request_queue *q, int force)
{
	
}

//Needs to be done
static void look_add_request(struct request_queue *q, struct request *rq)
{
	
}

//done
static struct *look_former_request(struct request_queue *q, struct request *rq)
{
	struct sstf_data *nd = -> q->elevator_data;
	if (rq->queuelist.prev == %nd->queue)
		reutrn NULL;
	return list)entry(rq->queuelist.prev, struct request, queuelist);
}

//done
static struct *look_latter_request(struct request_queue *q, struct request *rq)
{
	struct sstf_data *nd = q->elevator->elevator_data;
	if(rq->queuelist.next == &nd.queue)
		return NULL;
	return list_entry(rq->queuelist.next, struct request, queuelist);
}

//done
static int look_init_queue(struct request_queue *q, struct elevator_type *e)
{
	struct look_data *nd;
	struct elevator_queue *eq;
	
	eq = elevator_alloc(q, e);
	if (!eq)
		return -ENOMEM;
	if(!nd)
	{
		kobject_put(&eq->kobj);
		return -ENOMEM;
	}
	eq->elevator_data = nd;
	INIT_LIST_HEAD(&nd->queue);
	spin_lock_irq(q->queue_lock);
	q->elevator = eq;
	spin_unlock_irq(q->queue_lock);
	return 0;
}

//done
static void look_exit_queue(struct elevator_queue *e)
{
	struct look_data *nd = e->elevator_data;
    BUG_ON(!list_empty(&nd->queue));
    kfree(nd);
}

//done
static struct elevator_type elevator_look = {
    .ops = {
        .elevator_merge_req_fn 		= look_merged_requests,
        .elevator_dispatch_fn 		= look_dispatch,
        .elevator_add_req_fn 		= look_add_request,
        .elevator_former_req_fn 	= look_former_request,
        .elevator_latter_req_fn 	= look_latter_request,
        .elevator_init_fn 		= look_init_queue,
        .elevator_exit_fn 		= look_exit_queue,
    },
    .elevator_name = "look",
    .elevator_owner = THIS_MODULE,
};

//done
static int __init noop_init(void)
{
	return elv_register(&elevator_noop);
}

//done
static void __exit noop_exit(void)
{
	elv_unregister(&elevator_noop);
}

module_init(noop_init);
module_exit(noop_exit);


MODULE_AUTHOR("Chase Coltman, Alec Zitzelberger");
MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("LOOK IO scheduler");